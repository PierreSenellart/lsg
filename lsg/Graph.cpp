/* $Id: Graph.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include <ios>
#include <stdexcept>
#include <cstdio>
#include <map>
#include <cassert>
#include <algorithm>

#include "unistd.h"

#include "MutableGraph.h"
#include "Tools.h"

using namespace std;

namespace lsg {
  ostream &operator<<(ostream &out,const Graph &g)
  {
    const node_t size=g.getNbNodes();
    const bool with_values=g.hasValues();

    streamsize oldPrecision=out.precision(20);
    out << size << "\n";
    out << (with_values?"with values":"no values") << "\n";
    for(node_t i=0;i<size;++i) {
      out << i;

      for(SparseArray::const_iterator it=g.row(i).begin(),itend=g.row(i).end();
          it!=itend;
          ++it) {
        out << " " << it.index();
        if(with_values)
          out << "," << *it;
      }

      out << "\n";
    }  

    out.precision(oldPrecision);
    return out;
  }
  
  bool operator==(const Graph &g, const Graph &h)
  {
    node_t size=g.getNbNodes();
    if(size!=h.getNbNodes())
      return false;

    for(node_t i=0;i<size;++i) {
      SparseArray::const_iterator git=g.row(i).begin(),
                                  gitend=g.row(i).end(),
                                  hit=h.row(i).begin(),
                                  hitend=h.row(i).end();

      for(;git!=gitend && hit!=hitend;++git,++hit) {
        while(git!=gitend && *git==0) ++git;

        while(hit!=hitend && *hit==0) ++hit;

        if(git==gitend || hit==hitend)
          break;

        if(git.index()!=hit.index() || *hit!=*git)
          return false;
      }

      if(git==gitend)
        while(hit!=hitend && *hit==0) ++hit;

      if(hit==hitend)
        while(git!=gitend && *git==0) ++git;

      if(hit!=hitend || git!=gitend)
        return false;
    }

    return true;
  }

  FILE *Graph::beginStore(const string &filename, node_t size) const
  {
    FILE *f=fopen(filename.c_str(),"w");
    if(ftruncate(fileno(f),0))
      return 0;
    fseek(f,0,SEEK_SET);
    
    if(!f)
      return 0;
    
    fwrite("GPH",1,3,f);

    unsigned char charmagic=
      MAGIC_WITH_TRANSPOSE |
      (hasValues()?MAGIC_WITH_VALUES:0) |
      (hasLabels()?MAGIC_WITH_LABELS:0);

    fwrite(&charmagic,1,1,f);

    seekTillAlign(f,sizeof(node_t));
    fwrite(&size,sizeof(node_t),1,f);

    node_t nbEdges=getNbEdges();
    fwrite(&nbEdges,sizeof(node_t),1,f);

    node_t offsetr=0,offsetc=0,offsetl=0;

    bool with_values=hasValues();

    for(node_t i=0;i<size;++i) {
      fwrite(&offsetr,sizeof(node_t),1,f);
      offsetr+=1+row(i).size()*(with_values?2:1);
    }  

    for(node_t i=0;i<size;++i) {
      fwrite(&offsetc,sizeof(node_t),1,f);
      offsetc+=1+column(i).size()*(with_values?2:1);
    }

    if(hasLabels()) {
      for(node_t i=0;i<size;++i) {
        fwrite(&offsetl,sizeof(node_t),1,f);
        offsetl+=1+getLabelSize(i);
      }
    }

    return f;
  }

  struct CompareByFirst :
    std::binary_function<
    const pair<node_t,node_t> &,
    const pair<node_t,node_t> &,
    bool>
  {
    inline bool operator()(
        const pair<node_t,node_t> &lhs,
        const pair<node_t,node_t> &rhs)
    {
      return lhs.first<rhs.first;
    }
  };

  bool Graph::storeSubgraph(const std::string &filename,
                            const std::vector<bool> &nodes) const
  {
    if(!hasValues())
      throw std::logic_error("Not implemented.");

    node_t newSize=count(nodes.begin(),nodes.end(),true);

    FILE *f=beginStore(filename,newSize);
    // The number of edges, offsets are
    // incorrect, this will be fixed at the end of this function

    if(!f)
      return false;

    std::vector<node_t> rowSize(newSize);
    std::vector<node_t> columnSize(newSize);

    node_t oldSize=getNbNodes();
  
    vector<node_t> reindex(oldSize);
    node_t k=0;
    
    for(node_t i=0;i<oldSize;++i)
      if(nodes[i])
        reindex[i]=k++;

    node_t nbEdges=0;

    vector<vector<pair<node_t,node_t> > > edges(newSize);

    for(node_t i=0;i<oldSize;++i) {
      if(!nodes[i])
        continue;

      const node_t reindex_i=reindex[i];
      const unsigned int start=ftell(f);

      fseek(f,sizeof(node_t),SEEK_CUR); // We'll change this at the end
                                         // of the loop

      node_t nb=0;

      for(SparseArray::const_iterator it=row(i).begin(),
                                   itend=row(i).end();
          it!=itend;
          ++it) {
        if(nodes[it.index()]) {
          const node_t j=reindex[it.index()];

          edges[reindex_i].push_back(make_pair(j,nbEdges));

          fwrite(&j,sizeof(node_t),1,f);
          fwrite(&nbEdges,sizeof(node_t),1,f);

          ++nb,++nbEdges;
        }
      }

      sort(edges[reindex_i].begin(),edges[reindex_i].end(),CompareByFirst());

      const unsigned int end=ftell(f);
      fseek(f,start,SEEK_SET);
      fwrite(&nb,sizeof(node_t),1,f);
      fseek(f,end,SEEK_SET);

      rowSize[reindex[i]]=nb;
    }

    for(node_t i=0;i<oldSize;++i) {
      if(!nodes[i])
        continue;

      const node_t reindex_i=reindex[i];
      const unsigned int start=ftell(f);

      fseek(f,sizeof(node_t),SEEK_CUR); // We'll change this at the end
                                         // of the loop

      node_t nb=0;
      
      for(SparseArray::const_iterator it=column(i).begin(),
                                   itend=column(i).end();
          it!=itend;
          ++it) {
        if(nodes[it.index()]) {
          const node_t j=reindex[it.index()];
          
          const node_t valueIndex=
            lower_bound(edges[j].begin(),edges[j].end(),
                        make_pair(reindex_i,0),
                        CompareByFirst())->second;
          
          fwrite(&j,sizeof(node_t),1,f);
          fwrite(&valueIndex,sizeof(node_t),1,f);

          ++nb;
        }
      }
      
      const unsigned int end=ftell(f);
      fseek(f,start,SEEK_SET);
      fwrite(&nb,sizeof(node_t),1,f);
      fseek(f,end,SEEK_SET);

      columnSize[reindex[i]]=nb;
    }

    // We can free up the space used by edges now !
    edges.clear();

    seekTillAlign(f,sizeof(value_t));

    for(node_t i=0;i<oldSize;++i) {
      if(!nodes[i])
        continue;

      for(SparseArray::const_iterator it=row(i).begin(),
                                   itend=row(i).end();
          it!=itend;
          ++it)
        if(nodes[it.index()])
          fwrite(&*it,sizeof(value_t),1,f);
    }
    
    if(hasLabels())
      for(node_t i=0;i<oldSize;++i) {
        if(nodes[i])
          fwrite(getLabel(i).c_str(),1,getLabelSize(i)+1,f);
      }

    // Let's now write the proper values for edges and offsets
    fseek(f,4,SEEK_SET);
    seekTillAlign(f,sizeof(node_t)); 
    fseek(f,sizeof(node_t),SEEK_CUR);
    
    fwrite(&nbEdges,sizeof(node_t),1,f);
    
    node_t offsetr=0,offsetc=0,offsetl=0;

    bool with_values=true;

    for(node_t i=0;i<newSize;++i) {
      fwrite(&offsetr,sizeof(node_t),1,f);
      offsetr+=1+rowSize[i]*(with_values?2:1);
    }  

    for(node_t i=0;i<newSize;++i) {
      fwrite(&offsetc,sizeof(node_t),1,f);
      offsetc+=1+columnSize[i]*(with_values?2:1);
    }
    
    if(hasLabels())
      for(node_t i=0;i<oldSize;++i)
        if(nodes[i]) {
          fwrite(&offsetl,sizeof(node_t),1,f);
          offsetl+=1+getLabelSize(i);
        }

    return !fclose(f);
  }

  bool Graph::storeWithTransposedEdges(const string &filename) const
  {
    if(!hasValues())
      throw std::logic_error("Not implemented.");

    node_t size=getNbNodes();

    FILE *f=beginStore(filename,size); 
    // The number of edges and offsets are incorrect, this will be fixed
    // at the end of this function

    if(!f)
      return false;
  
    node_t nbEdges=getNbEdges();

    std::vector<node_t> rowSize(size);
    std::vector<node_t> columnSize(size);
    
    vector<vector<pair<node_t,node_t> > > edges(size);

    for(node_t i=0;i<size;++i) {
      unsigned int start=ftell(f);

      fseek(f,sizeof(node_t),SEEK_CUR); // We'll change this at the end
                                         // of the loop

      node_t nb=0;
      const SparseArray &r=row(i),&c=column(i);

      SparseArray::const_iterator
        itr=r.begin(),
        itendr=r.end(),
        itc=c.begin(),
        itendc=c.end();

      for(;itr!=itendr || itc!=itendc;) {
        ++nb;

        if(itc==itendc || (itr!=itendr && itr.index()<itc.index())) {
          itr.write(f);
          ++itr;
        } else if(itr==itendr || itr.index()>itc.index()) {
          node_t j=itc.index();
          fwrite(&j,sizeof(node_t),1,f);
          fwrite(&nbEdges,sizeof(node_t),1,f);

          edges[i].push_back(make_pair(j,nbEdges));
          ++nbEdges,++itc;
        } else {
          itr.write(f);
          ++itr,++itc;
        }
      }

      sort(edges[i].begin(),edges[i].end(),CompareByFirst());

      unsigned int end=ftell(f);
      fseek(f,start,SEEK_SET);
      fwrite(&nb,sizeof(node_t),1,f);
      fseek(f,end,SEEK_SET);

      rowSize[i]=nb;
    }

    for(node_t i=0;i<size;++i) {
      unsigned int start=ftell(f);
      
      fseek(f,sizeof(node_t),SEEK_CUR); // We'll change this at the end
                                         // of the loop
      
      node_t nb=0;
      const SparseArray &r=row(i),&c=column(i);

      SparseArray::const_iterator
        itr=r.begin(),
        itendr=r.end(),
        itc=c.begin(),
        itendc=c.end();

      for(;itr!=itendr || itc!=itendc;) {
        ++nb;

        if(itc==itendc || (itr!=itendr && itr.index()<itc.index())) {
          node_t j=itr.index();
          fwrite(&j,sizeof(node_t),1,f);
          
          const node_t valueIndex=
            lower_bound(edges[j].begin(),edges[j].end(),
                        make_pair(i,0),
                        CompareByFirst())->second;
          
          fwrite(&valueIndex,sizeof(node_t),1,f);

          ++itr;
        } else if(itr==itendr || itr.index()>itc.index()) {
          itc.write(f);
          ++itc;
        } else {
          itc.write(f);
          ++itr,++itc;
        }
      }

      unsigned int end=ftell(f);
      fseek(f,start,SEEK_SET);
      fwrite(&nb,sizeof(node_t),1,f);
      fseek(f,end,SEEK_SET);
      
      columnSize[i]=nb;
    }

    seekTillAlign(f,sizeof(value_t));

    writeValues(f);

    value_t dummy=0;

    node_t additional_edges=nbEdges-getNbEdges();
    for(node_t i=0;i<additional_edges;++i)
      fwrite(&dummy,sizeof(value_t),1,f);

    if(hasLabels())
      for(node_t i=0;i<size;++i) {
        fwrite(getLabel(i).c_str(),1,getLabelSize(i)+1,f);
      }

    // Let's now write the proper values for edges and offsets
    fseek(f,4,SEEK_SET);
    seekTillAlign(f,sizeof(node_t)); 
    fseek(f,sizeof(node_t),SEEK_CUR);
    
    fwrite(&nbEdges,sizeof(node_t),1,f);
    
    node_t offsetr=0,offsetc=0;

    bool with_values=true;

    for(node_t i=0;i<size;++i) {
      fwrite(&offsetr,sizeof(node_t),1,f);
      offsetr+=1+rowSize[i]*(with_values?2:1);
    }  

    for(node_t i=0;i<size;++i) {
      fwrite(&offsetc,sizeof(node_t),1,f);
      offsetc+=1+columnSize[i]*(with_values?2:1);
    }

    return !fclose(f);
  }

  bool Graph::store(const string &filename) const
  {
    node_t size=getNbNodes();

    FILE *f=beginStore(filename,size);

    if(!f)
      return false;
    
    for(node_t i=0;i<size;++i)
      row(i).write(f);

    for(node_t i=0;i<size;++i)
      column(i).write(f);

    if(hasValues()) {
      seekTillAlign(f,sizeof(value_t));

      writeValues(f);
    }

    if(hasLabels())
      for(node_t i=0;i<size;++i) {
        fwrite(getLabel(i).c_str(),1,getLabelSize(i)+1,f);
      }

    return !fclose(f);
  }

  value_t Graph::operator()(node_t i,node_t j) const {
    assert(i<getNbNodes());
    assert(j<getNbNodes());

    SparseArray::const_iterator it=row(i).find(j);

    if(it!=row(i).end())
      return *it;
    else
      return 0;
  }

  value_t &Graph::Proxy::operator+=(const value_t &v)
  {
    SparseArray::iterator it=graph.row(r).find(c);

    if(it!=graph.row(r).end())
      return *it+=v;
    else
      return graph.insert_new_edge(r,c)+=v;
  }

  value_t &Graph::Proxy::operator*=(const value_t &v)
  {
    SparseArray::iterator it=graph.row(r).find(c);

    if(it!=graph.row(r).end())
      return *it*=v;
    else
      return graph.insert_new_edge(r,c)*=v;
  }

  value_t &Graph::Proxy::operator=(const value_t &v)
  {
    SparseArray::iterator it=graph.row(r).find(c);

    if(it!=graph.row(r).end())
      return *it=v;
    else
      return graph.insert_new_edge(r,c)=v;
  }
  
  Graph &Graph::operator*=(value_t v)
  {
    node_t size=getNbNodes();

    for(node_t i=0;i<size;++i)
      for(SparseArray::iterator it=row(i).begin(),
                             itend=row(i).end();
          it!=itend;
          ++it)
        *it *= v;

    return *this;
  }
}
