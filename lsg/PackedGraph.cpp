/* $Id: PackedGraph.cpp 8147 2013-07-09 08:29:13Z pierre $ */

/*
 *  Copyright (c) 2006 Yann Ollivier <yann.ollivier@normalesup.org>
 *                     Pierre Senellart <pierre@senellart.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit
 *  persons to whom the Software is furnished to do so, subject to the
 *  following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 *  NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 *  USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>
#include <cassert>
#include <cstring>

#include "SparseArray.h"
#include "PackedGraph.h"
#include "Tools.h"

using namespace std;

namespace lsg {
  PackedGraph::PackedGraph(const string &filename)
  {
    fd=open(filename.c_str(),O_RDWR);

    if(fd==-1)
      return;

    char magic[4];
    if(read(fd,magic,4)!=4)
      return;
    if(strncmp(magic,"GPH",3))
      return;

    with_values=(magic[3]&MAGIC_WITH_VALUES);
    with_transpose=(magic[3]&MAGIC_WITH_TRANSPOSE);
    with_labels=(magic[3]&MAGIC_WITH_LABELS);
    is_transposed=(magic[3]&MAGIC_IS_TRANSPOSED);

    seekTillAlign(fd,sizeof(node_t));

    if(read(fd,&size,sizeof(node_t))!=sizeof(node_t))
      return;

    if(read(fd,&nbEdges,sizeof(node_t))!=sizeof(node_t))
      return;

    off_t position=lseek(fd,0,SEEK_CUR);
    filesize=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    mmaped_region=mmap(0,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(mmaped_region==MAP_FAILED)
      return;

    indexr=reinterpret_cast<node_t *>(
        reinterpret_cast<char *>(mmaped_region)+position);

    if(with_transpose)
      indexc=indexr+size;
    if(with_labels)
      indexl=(with_transpose?indexc:indexr)+size;

    rows=const_cast<node_t*>(
        indexr+(1+(with_transpose?1:0)+(with_labels?1:0))*size);

    if(with_transpose)
      columns=rows+nbEdges*(with_values?2:1)+size;

    if(with_values) {
      char *pos=reinterpret_cast<char *>
        (((with_transpose?columns:rows)+nbEdges*2+size));

      ptrdiff_t offset=
        sizeof(value_t)-
        (pos-reinterpret_cast<char *>(mmaped_region))%sizeof(value_t);
      
      if(offset==sizeof(value_t))
        offset=0;

      values=reinterpret_cast<value_t *>(pos+offset);
      labels=reinterpret_cast<char*>(values+nbEdges);
    } else if(with_labels)
      labels=reinterpret_cast<char*>((with_transpose?columns:rows)+
          nbEdges*(with_values?2:1)+size);

    init_sparse_arrays();

    if(is_transposed)
      swap_rows_columns();
      
    setOk();
  }

  void PackedGraph::swap_rows_columns()
  {
    node_t * const temp=rows;
    rows=columns;
    columns=temp;

    const node_t* const temp2=indexr;
    indexr=indexc;
    indexc=temp2;

    vector<PGSparseArray> *const temp3=sparse_rows;
    sparse_rows=sparse_columns;
    sparse_columns=temp3;
  }

  PackedGraph::~PackedGraph()
  {
    destroy();

    delete sparse_columns;
    delete sparse_rows;
  }

  void PackedGraph::destroy() {
    if(!destroyed) {
      if(mmaped_region!=MAP_FAILED)
        munmap(mmaped_region,filesize);
      if(fd!=-1)
        close(fd);
      destroyed=true;
    }
  }

  value_t &PackedGraph::insert_new_edge(node_t,node_t) {
    throw domain_error("Cannot insert a new edge in a PackedGraph");
  }

  void PackedGraph::transpose()
  {
    if(!with_transpose)
      throw domain_error("No transposition available");

    swap_rows_columns();

    is_transposed=!is_transposed;
    reinterpret_cast<char*>(mmaped_region)[3]^=MAGIC_IS_TRANSPOSED;
  }

  string PackedGraph::getLabel(node_t i) const
  {
    if(!with_labels)
      return "";

    return labels+indexl[i];
  }

  size_t PackedGraph::getLabelSize(node_t i) const
  {
    if(!with_labels)
      return 0;

    return strlen(labels+indexl[i]);
  }

  node_t PackedGraph::getNodeWithLabel(const string &s) const
  {
    if(with_labels) {
      const char *chaine=s.c_str();

      for(node_t i=0;i<size;++i) {
        if(!strcmp(labels+indexl[i],chaine))
          return i;
      }
    }

    return static_cast<node_t>(-1);
  }

  void PackedGraph::init_sparse_arrays()
  {
    sparse_rows=new vector<PGSparseArray>(size);
    sparse_columns=new vector<PGSparseArray>(size);

    for(node_t i=0;i<size;++i) {
      (*sparse_rows)[i].set(values,rows+indexr[i]);
      (*sparse_columns)[i].set(values,columns+indexc[i]);
    }
  }

  void PackedGraph::writeValues(FILE *f) const
  {
    fwrite(values,sizeof(value_t),nbEdges,f);
  }
    
  void PGSparseArray::write(FILE *f) const
  {
    fwrite(start,sizeof(node_t),1+2*size(),f);
  }
    
  SparseArray::iterator PGSparseArray::find(node_t index)
  {
    node_t s=*start;
    const node_t *p=start+1,*pend=start+1+2*s;
    for(;p<pend;p+=2) {
      if(*p==index)
        break;
    }
    
    return buildIterator(p);
  }
 
  SparseArray::const_iterator PGSparseArray::find(node_t index) const
  {
    node_t s=*start;
    const node_t *p=start+1,*pend=start+1+2*s;
    for(;p<pend;p+=2) {
      if(*p==index)
        break;
    }

    return buildConstIterator(p);
  }
}
