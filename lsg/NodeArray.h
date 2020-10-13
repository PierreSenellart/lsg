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

#ifndef NODEARRAY_H
#define NODEARRAY_H

#include <map>

#include "SparseArray.h"

namespace lsg {
  class Graph;
  
  template<typename Value> struct NAValueTraits {
    typedef std::map<node_t,value_t>::iterator iterator;
  };

  template<typename Value> struct NAValueTraits<const Value> {
    typedef std::map<node_t,value_t>::const_iterator iterator;
  };

  template<typename Value>
    class NodeArrayIterator : public ISparseArrayIterator<Value> {
   public:
    NodeArrayIterator(const typename NAValueTraits<Value>::iterator &i) : it(i)
      {}
    virtual ~NodeArrayIterator() { };

    // Methods inherited from ISparseArrayIterator
    inline virtual ISparseArrayIterator<Value> *clone() const
      { return new NodeArrayIterator(*this); }
    inline virtual Value &operator*() const
      { return it->second; }
    inline virtual void operator++()
      { ++it; }
    inline virtual void write(FILE *) const { /* N/A */ }
    virtual bool operator==(const ISparseArrayIterator<Value> &sai) const {
      const NodeArrayIterator *nai=
        dynamic_cast<const NodeArrayIterator*>(&sai);
      if(!nai)
        return 0;
      else
        return it==nai->it;
    }
    inline virtual node_t index() const
      { return it->first; }

    // Other methods
    inline bool operator==(const NodeArrayIterator nai) const
      { return it==nai.it; }

   private:
    typename NAValueTraits<Value>::iterator it;
  };

  class NodeArray : public SparseArray
  {
  public:
    // Constructors and destructor
    NodeArray(const SparseArray &sa);
		NodeArray() { }
    virtual ~NodeArray() { }

    // Methods inherited from SparseArray
    inline virtual SparseArray::iterator begin()
      { return buildIterator(m.begin()); }
    inline virtual SparseArray::const_iterator begin() const
      { return buildIterator(m.begin()); }

    inline virtual SparseArray::iterator end()
      { return buildIterator(m.end()); }
    inline virtual SparseArray::const_iterator end() const
      { return buildIterator(m.end()); }

    inline virtual SparseArray::iterator find(node_t index)
      { return buildIterator(m.find(index)); }
    inline virtual SparseArray::const_iterator find(node_t index) const
      { return buildIterator(m.find(index)); }

    virtual value_t operator[](node_t index) const;

    inline virtual node_t size() const
      { return m.size(); } 

    inline virtual void write(FILE *) const { /* N/A */ }

    // Other methods
    inline void insert(node_t index, value_t value)
      { m.insert(std::make_pair(index,value)); }
    inline void clear()
      { m.clear();}
		
		friend void addNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result);
		friend void multNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result);

  private:
    inline SparseArray::iterator
      buildIterator(const std::map<node_t,value_t>::iterator it) const {
      return new NodeArrayIterator<value_t>(it);
    }
    inline SparseArray::const_iterator
      buildIterator(const std::map<node_t,value_t>::const_iterator it) const {
      return new NodeArrayIterator<const value_t>(it);
    }

    std::map<node_t,value_t> m;
  };

  void directedSphere
    (const Graph &g, node_t node, const std::string &direction,
     NodeArray &result);
		
	void AddNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result);
	void MultNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result);
}

#endif /* NODEARRAY_H */
