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

#include "unistd.h"

#include <ios>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <functional>
#include <sstream>
#include <iostream>
#include <limits>

#include "MutableGraph.h"

using namespace std;

namespace lsg {
  MutableGraph::MutableGraph(istream &in)
  {
    size=0;
    in >> *this;
  }

  istream &operator>>(istream &in, MutableGraph &g)
  {
    g.destroy();
    g.destroyed=false;
    g.with_labels=false;

    node_t size;
    in >> size;
    g.initialize(size);
    
    MutableGraph::BatchInsertor bi(g);

    in.ignore(numeric_limits<streamsize>::max(),'\n');
    std::string s;
    getline(in,s);
    bool with_values=(s=="with values");

    node_t i;
    while(in >> i) {
      getline(in,s);
      stringstream ss(s);

      node_t j;
      while(ss >> j) {
        value_t value=1.;
        if(with_values) {
          ss.ignore(numeric_limits<streamsize>::max(),',');
          ss >> value;
        }
        bi.add(i,j,value);
      }
    }

    return in;
  }

  MutableGraph::MutableGraph(node_t nbNodes) {
    with_labels=false;

    initialize(nbNodes);
  }

  MutableGraph::MutableGraph(const Graph &g,const vector<bool> &nodes)
  {
    with_labels=g.hasLabels();
    
    initialize(count(nodes.begin(),nodes.end(),true));

    node_t gSize=g.getNbNodes();
    
    vector<node_t> reindex(gSize);
    node_t k=0;
    
    for(node_t i=0;i<gSize;++i) {
      if(nodes[i]) {
        if(with_labels)
          labels[k]=g.getLabel(i);

        reindex[i]=k++;
      }
    }

    BatchInsertor bi(*this);
    for(node_t i=0;i<gSize;++i) {
      if(nodes[i]) {
        for(SparseArray::const_iterator it=g.row(i).begin(),
            itend=g.row(i).end();
            it!=itend;
            ++it) {
          if(nodes[it.index()])
            bi.add(reindex[i],reindex[it.index()],*it);
        }
      }
    }
  }

  void MutableGraph::initialize(node_t nbNodes) {
    setOk();
    size=nbNodes;
    rows=new vector<MGSparseArray*>(size);
    columns=new vector<MGSparseArray*>(size);

    for(vector<MGSparseArray*>::iterator it=rows->begin(),
                                     itend=rows->end();
        it!=itend;
        ++it)
      *it=new MGSparseArray(values);
    
    for(vector<MGSparseArray*>::iterator it=columns->begin(),
                                     itend=columns->end();
        it!=itend;
        ++it)
      *it=new MGSparseArray(values);

    labels.resize(with_labels?size:0);
  }

  void MutableGraph::destroy() { 
    if(!destroyed) {
      for(node_t i=0;i<size;++i) {
        delete (*rows)[i];
        delete (*columns)[i];
      }
      destroyed=true;
    }
  }

  value_t &MutableGraph::insert_new_edge(node_t i,node_t j) {
    SparseArray::iterator it = (*rows)[i]->find(j);

    if(it==(*rows)[i]->end()) {
      values.push_back(value_t());
      it=(*rows)[i]->insert(j,values.size()-1);
      (*columns)[j]->insert(i,values.size()-1);
    }

    return *it;
  }

  void MutableGraph::remove(node_t i,node_t j)
  {
    assert(i<size);
    assert(j<size);

    SparseArray::iterator it=row(i).find(j);
    
    if(it!=row(i).end()) {
      // Let's just set this cell to 0, removal would be too costly
      // since it would require to reindex all existing values
      *it=value_t();

      (*rows)[i]->erase(it);
      (*columns)[i]->remove(j);
    }
  }

  MutableGraph &MutableGraph::operator+=(const Graph &g)
  {
    node_t size=g.getNbNodes();
    
    for(node_t i=0;i<size;++i) {
      for(SparseArray::const_iterator it=g.row(i).begin(),
                                   itend=g.row(i).end();
          it!=itend;
          ++it) {
        (*this)(i,it.index())+=*it;
      }
    }
    
    return *this;
  }

  MutableGraph &MutableGraph::operator|=(const Graph &g)
  {
    node_t size=g.getNbNodes();
    
    BatchInsertor bi(*this);
    for(node_t i=0;i<size;++i) {
      for(SparseArray::const_iterator it=g.row(i).begin(),
                                itend=g.row(i).end();
          it!=itend;
          ++it) {
        bi.add(i,it.index(),*it);
      }
    }
    
    return *this;
  }

  MutableGraph::MutableGraph(const Graph &g)
  {
    copy(g);
  }

  void MutableGraph::copy(const Graph &g)
  {
    if(g.isOk()) {
      with_labels=g.hasLabels();

      initialize(g.getNbNodes());

      BatchInsertor bi(*this);
      for(node_t i=0;i<size;++i) {
        for(SparseArray::const_iterator it=g.row(i).begin(),
            itend=g.row(i).end();
            it!=itend;
            ++it) {
          bi.add(i,it.index(),*it);
        }

        if(with_labels)
          labels[i]=g.getLabel(i);
      }
    }
  }

  MutableGraph &MutableGraph::operator=(const Graph &g)
  {
    if(&g==this)
      return *this;

    destroy();
    destroyed=false;
    copy(g);

    return *this;
  }

  void MutableGraph::copy(const MutableGraph &g)
  {
    if(g.isOk()) {
      size=g.size;
      rows->resize(size);
      columns->resize(size);

      for(node_t i=0;i<size;++i)
        (*rows)[i]=new MGSparseArray
          (*(*g.rows)[i],values);

      for(node_t j=0;j<size;++j)
        (*columns)[j]=new MGSparseArray
          (*(*g.columns)[j],values);

      values=g.values;
      with_labels=g.with_labels;
      labels=g.labels;
    }
  }

  MutableGraph::MutableGraph(const MutableGraph &g) : Graph(g)
  {
    if(g.isOk()) {
      with_labels=g.hasLabels();
      initialize(g.size);
      destroy();
      destroyed=false;
      copy(g);
    }
  }

  MutableGraph &MutableGraph::operator=(const MutableGraph &g)
  {
    if(&g==this)
      return *this;

    destroy();
    destroyed=false;
    copy(g);

    return *this;
  }

  MutableGraph RandomGraph(node_t nbNodes, double p)
  {
    MutableGraph g(nbNodes);

    MutableGraph::BatchInsertor bi(g);

    for(node_t i=0;i<nbNodes;++i)
      for(node_t j=0;j<nbNodes;++j)
        if(rand()<p*RAND_MAX)
          bi.add(i,j,1.);

    return g;
  }

  void MutableGraph::transpose()
  {
    vector<MGSparseArray*> *temp=rows;
    rows=columns;
    columns=temp;
  }

  string MutableGraph::getLabel(node_t i) const
  {
    static const string empty_string="";

    assert(i<size);

    if(!with_labels)
      return empty_string;

    return labels[i];
  }

  void MutableGraph::setLabel(node_t i,const string &s)
  {
    assert(i<size);

    if(!with_labels) {
      with_labels=true;
      labels.resize(size);
    }

    labels[i]=s;
  }

  string::size_type MutableGraph::getLabelSize(node_t i) const
  {
    assert(i<size);

    if(!with_labels)
      return 0;

    return labels[i].size();
  }

  node_t MutableGraph::getNodeWithLabel(const string &s) const
  {
    vector<string>::const_iterator it=find(labels.begin(),labels.end(),s);

    if(it==labels.end())
      return static_cast<node_t>(-1);
    else 
      return it-labels.begin();
  }

  void MutableGraph::consolidate()
  {
    for_each(rows->begin(),rows->end(), 
             mem_fun(&MGSparseArray::consolidate));
    for_each(columns->begin(),columns->end(), 
             mem_fun(&MGSparseArray::consolidate));
  }

  void MutableGraph::writeValues(FILE *f) const
  {
    node_t size=values.size();

    for(node_t i=0;i<size;++i)
      fwrite(&values[i],sizeof(value_t),1,f);
  }

  struct AddSize : binary_function<node_t,
                                   MGSparseArray *, 
                                   node_t> {
    inline node_t operator()(node_t sum, MGSparseArray *array)
    {
      return sum+array->size();
    }
  };

  node_t MutableGraph::getNbEdges() const
  {
    return values.size();
  }

  void MutableGraph::BatchInsertor::add(node_t i, node_t j, value_t value)
  {
    if((*graph.rows)[i]->find(j)==(*graph.rows)[i]->end()) {
      graph.values.push_back(value);

      node_t edge=graph.values.size()-1;

      (*graph.rows)[i]->insert_unordered(j,edge);
      (*graph.columns)[j]->insert_unordered(i,edge);
    }
  }

  SparseArray::iterator MGSparseArray::begin()
  {
    return buildIterator(vec.begin());
  }

  SparseArray::const_iterator MGSparseArray::begin() const
  {
    return new MGSparseArrayIterator<const value_t>(values,vec.begin());  
  }

  SparseArray::iterator MGSparseArray::end()
  {
    return new MGSparseArrayIterator<value_t>(values,vec.end());  
  }

  SparseArray::const_iterator MGSparseArray::end() const
  {
    return new MGSparseArrayIterator<const value_t>(values,vec.end());  
  }

  SparseArray::iterator MGSparseArray::find(node_t index)
  {
    pair<vec_t::iterator,vec_t::iterator> range=
      equal_range(vec.begin(),vec.end(),make_pair(index,0),
          MGSparseArrayLess());

    if(range.first==range.second)
      return buildIterator(vec.end());
    else
      return buildIterator(range.first);
  }

  SparseArray::const_iterator MGSparseArray::find(node_t index) const
  {
    pair<vec_t::const_iterator,vec_t::const_iterator> range=
      equal_range(vec.begin(),vec.end(),make_pair(index,0),
          MGSparseArrayLess());

    if(range.first==range.second)
      return buildIterator(vec.end());
    else
      return buildIterator(range.first);
  }

  SparseArray::iterator MGSparseArray::insert(node_t index, node_t edge)
  {
    pair<vec_t::iterator,vec_t::iterator> range=
      equal_range(vec.begin(),vec.end(),make_pair(index,0),
                  MGSparseArrayLess());
    
    if(range.first==range.second)
      return insert(index, edge, range.first);
    else
      return buildIterator(range.first);
  }

  void MGSparseArray::erase(const SparseArray::iterator &sai)
  {
    MGSparseArrayIterator<value_t> *mgsai=
      dynamic_cast<MGSparseArrayIterator<value_t> *>(sai.get());

    if(!mgsai)
      return;

    vec.erase(mgsai->get());
  }

  void MGSparseArray::remove(node_t index)
  {
    pair<vec_t::iterator,vec_t::iterator> range=
      equal_range(vec.begin(),vec.end(),make_pair(index,0),
                  MGSparseArrayLess());
    
    if(range.first!=range.second)
      vec.erase(range.first);   
  }
    
  SparseArray::iterator MGSparseArray::insert(node_t index, node_t edge,
                                              const vec_t::iterator &it)
  {
    return buildIterator(vec.insert(it,make_pair(index,edge)));
  }

  void MGSparseArray::write(FILE *f) const
  {
    node_t size=vec.size();
    fwrite(&size,sizeof(node_t),1,f);
    for(node_t i=0;i<size;++i) {
      fwrite(&vec[i].first,sizeof(node_t),1,f);
      fwrite(&vec[i].second,sizeof(node_t),1,f);
    }
  }

  void MGSparseArray::consolidate()
  {
    sort(vec.begin(),vec.end(),MGSparseArrayLess());
    vec.erase(unique(vec.begin(),vec.end(),MGSparseArrayCompare()),vec.end());
  }
}
