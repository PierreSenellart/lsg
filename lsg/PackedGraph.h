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

#ifndef PACKED_GRAPH_H
#define PACKED_GRAPH_H

#include <string>
#include <vector>

#include "Graph.h"
#include "SparseArray.h"
#include "Uncopyable.h"

namespace lsg {
  template<typename Value> class PGSparseArrayIterator :
    public ISparseArrayIterator<Value>, private Uncopyable {
    Value *values;
    const node_t *it;

   public:
    virtual ~PGSparseArrayIterator() { }
    PGSparseArrayIterator(Value *v,
                          const node_t *i) :
      values(v), it(i) {}

    // Methods inherited from ISparseArrayIterator
    inline virtual ISparseArrayIterator<Value> *clone() const
      { return new PGSparseArrayIterator(values,it); }
    inline virtual Value &operator*() const { return values[*(it+1)]; }
    inline virtual void operator++() { it+=2; }
    inline virtual node_t index() const { return *it; }
    inline virtual void write(FILE *f) const
    {
      fwrite(it,sizeof(node_t),2,f);
    }
    
    virtual bool operator==(const ISparseArrayIterator<Value> &sai) const
    {
      const PGSparseArrayIterator *mgsai=
        dynamic_cast<const PGSparseArrayIterator*>(&sai);
      if(!mgsai)
        return 0;
      else
        return it==mgsai->it;
    }
    
    // Additionnal (more efficiant) equality operator
    inline bool operator==(const PGSparseArrayIterator &sai) const
      { return it==sai.it; }
  };
  
  class PGSparseArray: public SparseArray {
    value_t * values;
    const node_t * start;

    inline SparseArray::iterator buildIterator(const node_t *p) const
    {
      return new PGSparseArrayIterator<value_t>(values,p);
    }

    inline SparseArray::const_iterator buildConstIterator(const node_t * p)
      const
    {
      return new PGSparseArrayIterator<const value_t>(values,p);
    }

   public:
    // Constructors
    PGSparseArray() : values(0), start(0) { }
    PGSparseArray(value_t *v,node_t *p) : values(v), start(p) { }
    inline void set(value_t *v,node_t *p) { values=v; start=p; }

    // Destructor
    virtual ~PGSparseArray() { }
    
    // Methods inherited from SparseArray
    inline virtual SparseArray::iterator begin()
      { return buildIterator(start+1); }
    inline virtual SparseArray::const_iterator begin() const
      { return buildConstIterator(start+1); }

    inline virtual SparseArray::iterator end()
      { return buildIterator(start+1+2* *start); }
    inline virtual SparseArray::const_iterator end() const
      { return buildConstIterator(start+1+2* *start); }

    virtual SparseArray::iterator find(node_t index);
    virtual SparseArray::const_iterator find(node_t index) const;

    inline virtual node_t size() const { return *start; }
    
    virtual void write(FILE *f) const;
  };

  class PackedGraph: public Graph {
   public:
    PackedGraph(const std::string &filename);
    ~PackedGraph();

    inline virtual node_t getNbNodes() const { return size; }
    inline virtual bool hasLabels() const { return with_labels; }
    inline virtual bool hasValues() const { return with_values; }
    inline virtual node_t getNbEdges() const { return nbEdges; }
    
    virtual SparseArray &row(node_t i)
      { return (*sparse_rows)[i]; }
    virtual SparseArray &column(node_t j)
      { return (*sparse_columns)[j]; }
    virtual const SparseArray &row(node_t i) const
      { return (*sparse_rows)[i]; }
    virtual const SparseArray &column(node_t j) const
      { return (*sparse_columns)[j]; }
    
    virtual std::string getLabel(node_t i) const;
    virtual std::string::size_type getLabelSize(node_t i) const;
    virtual node_t getNodeWithLabel(const std::string &s) const;

    virtual void transpose();

    virtual void destroy();

   private:
    node_t size;
    bool with_labels;
    bool with_values;
    const node_t *indexr;
    const node_t *indexc;
    const node_t *indexl;
    node_t *rows;
    node_t *columns;
    value_t *values;
    const char *labels;
    node_t nbEdges;
    void *mmaped_region;
    off_t filesize;
    int fd;
    bool with_transpose;
    bool is_transposed;
    std::vector<PGSparseArray> *sparse_rows;
    std::vector<PGSparseArray> *sparse_columns;

    void init_sparse_arrays();
    void swap_rows_columns();

    virtual void writeValues(FILE *f) const;
    virtual value_t &insert_new_edge(node_t i,node_t j);
  };
}

#endif /* PACKED_GRAPH_H */
