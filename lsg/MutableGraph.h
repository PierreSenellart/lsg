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

#ifndef MUTABLE_GRAPH_H
#define MUTABLE_GRAPH_H

#include <string>
#include <vector>
#include <utility>

#include "lsg.h"

#include "Uncopyable.h"
#include "Graph.h"

namespace lsg {
  template<typename Value> struct MGValueTraits {
    typedef std::vector<std::pair<node_t,node_t> >::iterator iterator;
  };

  template<typename Value> struct MGValueTraits<const Value> {
    typedef std::vector<std::pair<node_t,node_t> >
      ::const_iterator iterator;
  };

  template<typename Value> class MGSparseArrayIterator :
    public ISparseArrayIterator<Value>, private Uncopyable {
    typedef std::pair<node_t,node_t> pair_t;

    std::vector<value_t> &values;
    typename MGValueTraits<Value>::iterator it;

   public:
    virtual ~MGSparseArrayIterator() { }
    MGSparseArrayIterator(std::vector<value_t> &v,
                          const typename MGValueTraits<Value>::iterator &i):
      values(v), it(i) {}

    // Methods inherited from ISparseArrayIterator
    inline virtual ISparseArrayIterator<Value> *clone() const
      { return new MGSparseArrayIterator(values,it); }
    inline virtual Value &operator*() const { return values[it->second]; }
    inline virtual void operator++() { ++it; }
    inline virtual node_t index() const { return it->first; }
    inline virtual void write(FILE *f) const
    {
      fwrite(&it->first,sizeof(node_t),1,f);
      fwrite(&it->second,sizeof(node_t),1,f);
    }
    
    virtual bool operator==(const ISparseArrayIterator<Value> &sai) const
    {
      const MGSparseArrayIterator *mgsai=
        dynamic_cast<const MGSparseArrayIterator*>(&sai);
      if(!mgsai)
        return 0;
      else
        return it==mgsai->it;
    }
    
    // Additionnal (more efficiant) equality operator
    inline bool operator==(const MGSparseArrayIterator &sai) const
      { return it==sai.it; }

    inline std::vector<pair_t>::iterator get() const { return it; }
  };

  class MGSparseArray : public SparseArray, private Uncopyable {
    typedef std::vector<std::pair<node_t,node_t> > vec_t;

    vec_t vec;
    std::vector<value_t> &values;

    inline SparseArray::iterator buildIterator(const vec_t::iterator &it) const
    {
      return new MGSparseArrayIterator<value_t>(values,it);
    }

    inline SparseArray::const_iterator buildIterator
      (const vec_t::const_iterator &it) const
    {
      return new MGSparseArrayIterator<const value_t>(values,it);
    }

    struct MGSparseArrayCompare :
      std::binary_function<
        const MGSparseArray::vec_t::value_type &,
        const MGSparseArray::vec_t::value_type&,
        bool>
    {
      inline bool operator()(const vec_t::value_type &lhs,
          const vec_t::value_type &rhs){
        return lhs.first==rhs.first;
      }
    };

    struct MGSparseArrayLess :
      std::binary_function<
        const MGSparseArray::vec_t::value_type &,
        const MGSparseArray::vec_t::value_type&,
        bool>
    {
      inline bool operator()(const vec_t::value_type &lhs,
          const vec_t::value_type &rhs){
        return lhs.first<rhs.first;
      }
    };

    SparseArray::iterator insert(node_t index, node_t edge,
                                 const vec_t::iterator &it);

   public:
    // Constructors
    MGSparseArray(std::vector<value_t> &v) : values(v) { }
    MGSparseArray(const MGSparseArray &sa, std::vector<value_t> &v) :
      vec(sa.vec), values(v) { }

    // Destructor
    virtual ~MGSparseArray() { }
    
    // Methods inherited from SparseArray
    virtual SparseArray::iterator begin();
    virtual SparseArray::const_iterator begin() const;

    virtual SparseArray::iterator end();
    virtual SparseArray::const_iterator end() const;

    virtual SparseArray::iterator find(node_t index);
    virtual SparseArray::const_iterator find(node_t index) const;

    inline virtual node_t size() const { return vec.size(); }
    
    virtual void write(FILE *f) const;
    
    // Other methods
    SparseArray::iterator insert(node_t index, node_t edge);
    void erase(const SparseArray::iterator &it);
    void remove(node_t index);
    inline void insert_unordered(node_t index, node_t edge)
      { vec.push_back(std::make_pair(index,edge)); }
    void consolidate();
  };
  
  class MutableGraph: public Graph {
    public:
      // Constructors
      MutableGraph(node_t nbNodes=0);
      MutableGraph(std::istream &is);
      MutableGraph(const Graph &g);
      MutableGraph(const Graph &g,const std::vector<bool> &nodes);
      MutableGraph(const MutableGraph &g);

      // Destructor
      inline virtual ~MutableGraph() { destroy(); delete rows; delete columns; }

      // Assignment operator
      MutableGraph &operator=(const Graph &g);
      MutableGraph &operator=(const MutableGraph &g);

      // Methods inherited from Graph
      inline virtual node_t getNbNodes() const { return size; }
      inline virtual bool hasLabels() const { return with_labels; }
      inline virtual bool hasValues() const { return true; }
      virtual node_t getNbEdges() const;

      inline virtual SparseArray &row(node_t i) { return *(*rows)[i]; }
      inline virtual SparseArray &column(node_t j) { return *(*columns)[j]; }
      inline virtual const SparseArray &row(node_t i) const { return *(*rows)[i]; }
      inline virtual const SparseArray &column(node_t j) const {return *(*columns)[j];}

      virtual std::string getLabel(node_t i) const;
      virtual void setLabel(node_t i,const std::string &s);
      virtual std::string::size_type getLabelSize(node_t i) const;
      virtual node_t getNodeWithLabel(const std::string &s) const;
      
      virtual void transpose();
      
      virtual void destroy();

      // Additional methods, not inherited from Graph
      class BatchInsertor {
       public:
        BatchInsertor(MutableGraph &g) : graph(g) {}
        ~BatchInsertor() { graph.consolidate(); }
        void add(node_t i, node_t j, value_t d);

       private:
        MutableGraph &graph;
      };

      void remove(node_t i,node_t j);
      MutableGraph &operator|=(const Graph &g);
      MutableGraph &operator+=(const Graph &g);
    
    private:
      node_t size;
      bool with_labels;

      std::vector<MGSparseArray*> *rows;
      std::vector<MGSparseArray*> *columns;
      std::vector<std::string> labels;
      std::vector<value_t> values;

      void initialize(node_t nbNodes);
      void copy(const MutableGraph &g);
      void copy(const Graph &g);
      void consolidate();

      virtual void writeValues(FILE *f) const;
      virtual value_t &insert_new_edge(node_t i,node_t j);
  
      friend std::istream &operator>>(std::istream &in, MutableGraph &g);
  };

  MutableGraph RandomGraph(node_t nbNodes, double p);
  std::istream &operator>>(std::istream &in, MutableGraph &g);
}

#endif /* MUTABLE_GRAPH_H */
