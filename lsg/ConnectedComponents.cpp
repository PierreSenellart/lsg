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

#include <list>
#include <utility>
#include <set>
#include <iostream>
#include <stdexcept>

#include "SparseArray.h"
#include "MutableGraph.h"

#include "ConnectedComponents.h"

using namespace std;

namespace lsg {
  void stronglyConnectedComponentsFirstDFS(
      const Graph &g,
      vector<node_t> &lastIndex)
  {
    list<pair<node_t,bool> > toBrowse;

    node_t size=g.getNbNodes();
    lastIndex.resize(size);

    for(node_t k=0;k<size;++k)
      lastIndex[k]=0;
    
    node_t node=0;

    node_t currentIndex=0;
    while(node<size) {
      toBrowse.push_back(make_pair(node,true));
      lastIndex[node]=static_cast<node_t>(-1);

      while(!toBrowse.empty()) {
        pair<node_t,bool> &item=toBrowse.back();
        int i=item.first;

        if(item.second) { // Start
          item.second=false;

          for(SparseArray::const_iterator it=g.row(i).begin(),
              itend=g.row(i).end();
              it!=itend;
              ++it)
            if(lastIndex[it.index()]==0) {
              lastIndex[it.index()]=static_cast<node_t>(-1); 
                                   // Tempory non-0 value

              toBrowse.push_back(make_pair(it.index(),true));
            }
        } else { // End
          toBrowse.pop_back();
          lastIndex[i]=++currentIndex;
        }
      } 

      for(;node<size && lastIndex[node];++node)
        ;
    }
  }

  struct OrderBySecondDec {
    int operator()(const pair<node_t,node_t> &a,
                  const pair<node_t,node_t> &b) {
      return a.second>b.second;
    }
  };
  
  void stronglyConnectedComponentsSecondDFS(
      const Graph &g,
      const vector<node_t> &lastIndex,
      vector<node_t> &comp)
  {
    list<node_t> toBrowse;
    
    set<pair<node_t,node_t>,OrderBySecondDec> orderedVertices;
    
    node_t size=g.getNbNodes();

    for(node_t k=0;k<size;++k)
      orderedVertices.insert(make_pair(k,lastIndex[k]));

    node_t currentCompIndex=0;
    
    for(set<pair<node_t,node_t>,OrderBySecondDec>::const_iterator
          it=orderedVertices.begin(),itend=orderedVertices.end();
        it!=itend;
        ++it) {
      node_t node=it->first;
      
      if(comp[node])
        continue;
      
      toBrowse.push_back(node);
      comp[node]=++currentCompIndex;

      while(!toBrowse.empty()) {
        int i=toBrowse.back();
        toBrowse.pop_back();

        for(SparseArray::const_iterator it=g.column(i).begin(),
            itend=g.column(i).end();
            it!=itend;
            ++it)
          if(comp[it.index()]==0) {
            comp[it.index()]=currentCompIndex;
            toBrowse.push_back(it.index());
          }
      } 
    }
  }

  void stronglyConnectedComponents(const Graph &g,vector<node_t> &comp)
  {
    node_t size=g.getNbNodes();

    comp.resize(size);

    for(node_t i=0;i<size;++i)
      comp[i]=0;

    vector<node_t> lastIndex;
    stronglyConnectedComponentsFirstDFS(g,lastIndex);

    stronglyConnectedComponentsSecondDFS(g,lastIndex,comp);
  }

  void weaklyConnectedComponentsBrowse(
      const Graph &g,
      vector<node_t> &components,
      node_t node,
      node_t nb)
  {
    list<node_t> toBrowse;

    components[node]=nb;
    toBrowse.push_back(node);

    while(!toBrowse.empty()) {
      node_t i=toBrowse.front();
      toBrowse.pop_front();

      for(SparseArray::const_iterator it=g.row(i).begin(),
          itend=g.row(i).end();
          it!=itend;
          ++it) {
        if(components[it.index()]==0) {
          components[it.index()]=nb;
          toBrowse.push_back(it.index());
        }
      }
    }
  }

  void weaklyConnectedComponents(const Graph &g,vector<node_t> &comp)
  {
    node_t size=g.getNbNodes();
    MutableGraph h=g;
    h.transpose();
    h|=g;

    comp.resize(size);

    fill(comp.begin(),comp.end(),0);

    node_t pos=0;
    for(int nb=1;;++nb) {
      weaklyConnectedComponentsBrowse(h,comp,pos,nb);

      node_t k;
      for(k=pos+1;k<size;++k) {
        if(comp[k]==0) {
          pos=k;
          break;
        }
      }

      if(k>=size)
        break;
    }
  }

  void contractedGraph(
      const Graph &g,
      const vector<node_t> &components,
      MutableGraph &h)
  {
    node_t size=g.getNbNodes();

    if(components.size()!=size)
      throw invalid_argument("size");
    
    node_t nb_components=0;

    for(node_t k=0;k<size;++k)
      if(components[k]>nb_components)
        nb_components=components[k];
    
    h=MutableGraph(nb_components);

    for(node_t i=0;i<size;++i)
      for(SparseArray::const_iterator it=g.row(i).begin(),itend=g.row(i).end();
          it!=itend;
          ++it)
        if(components[i]!=components[it.index()])
          h(components[i]-1,components[it.index()]-1)=1.;
  }
}
