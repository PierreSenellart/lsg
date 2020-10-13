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

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <iosfwd>
#include <cstdio>

#include "lsg.h"

#include "SparseArray.h"

namespace lsg {
  class Graph {
   private:
    class Proxy {
       Graph &graph;
       const node_t r;
       const node_t c;

      public:
       Proxy(Graph &g,node_t i,node_t j) : graph(g),r(i),c(j) { }
       value_t &operator=(const value_t &v);
       value_t &operator+=(const value_t &v);
       value_t &operator*=(const value_t &v);
       inline operator value_t() const
         { return static_cast<const Graph &>(graph)(r,c); }
    };

   public:
    Graph() : ok(false),destroyed(false) {}
    virtual ~Graph() {}

    inline bool isOk() const { return ok; }
    virtual bool hasValues() const=0;
    virtual bool hasLabels() const=0;
    virtual node_t getNbNodes() const=0;
    virtual node_t getNbEdges() const=0;
    value_t operator()(node_t i,node_t j) const;
    inline Proxy operator()(node_t i,node_t j) { return Proxy(*this,i,j); }

    virtual SparseArray &row(node_t i)=0;
    virtual SparseArray &column(node_t j)=0;
    virtual const SparseArray &row(node_t i) const=0;
    virtual const SparseArray &column(node_t j) const=0;

    virtual std::string getLabel(node_t i) const=0;
    virtual node_t getNodeWithLabel(const std::string &s) const=0;
    virtual std::string::size_type getLabelSize(node_t i) const=0;

    inline node_t outDegree(node_t i) const { return row(i).size(); }
    inline node_t inDegree(node_t j) const { return column(j).size(); }

    bool store(const std::string &filename) const;
    bool storeWithTransposedEdges(const std::string &filename) const;
    bool storeSubgraph(const std::string &filename,
                       const std::vector<bool> &vec) const;

    virtual void transpose()=0;

    virtual void destroy()=0; // Warning: a call to this function
                              // makes all further operations on the
                              // graph undefined

    virtual Graph &operator*=(value_t v);
    inline virtual Graph &operator/=(value_t v) { return *this *= 1/v; }
    
  private:
    bool ok;
    virtual void writeValues(FILE *f) const=0;
    virtual value_t &insert_new_edge(node_t i,node_t j)=0;
    FILE *beginStore(const std::string &filename,node_t size) const;

  protected:
    inline void setOk() { ok=true; }
    bool destroyed;
  };

  std::ostream &operator<<(std::ostream &os,const Graph &g);
  bool operator==(const Graph &g, const Graph &h);
  inline bool operator!=(const Graph &g, const Graph &h) { return !(g==h); }

  const unsigned char MAGIC_WITH_VALUES   =0x01;
  const unsigned char MAGIC_WITH_TRANSPOSE=0x02;
  const unsigned char MAGIC_WITH_LABELS   =0x04;
  const unsigned char MAGIC_IS_TRANSPOSED =0x08;
}

#endif /* GRAPH_H */
