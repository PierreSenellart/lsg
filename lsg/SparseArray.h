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

#ifndef SPARSE_ARRAY_H
#define SPARSE_ARRAY_H

#include <iterator>

#include "lsg.h"

namespace lsg {
  template<typename Value> class ISparseArrayIterator {
   public:
    virtual ~ISparseArrayIterator() { };

    virtual ISparseArrayIterator *clone() const=0;
    virtual Value &operator*() const=0;
    virtual void operator++()=0;
    virtual bool operator==(const ISparseArrayIterator &it) const=0;
    virtual node_t index() const=0;
    virtual void write(FILE *f) const=0;
  };

  template<typename Value> class SparseArrayIteratorTemplate :
    public std::iterator<std::bidirectional_iterator_tag, value_t> {
    ISparseArrayIterator<Value> * it;

   public:
    SparseArrayIteratorTemplate(ISparseArrayIterator<Value> *i): it(i) {}
    SparseArrayIteratorTemplate(const SparseArrayIteratorTemplate &sai): 
      it(sai.it->clone()) {}
    ~SparseArrayIteratorTemplate() { delete it; }

    ISparseArrayIterator<Value> *get() const { return it; }
    
    SparseArrayIteratorTemplate &operator=
      (const SparseArrayIteratorTemplate &sai)
    {
      if(&sai==this)
        return *this;

      ISparseArrayIterator<Value> *temp=sai.it->clone();

      delete it;
      it=temp;

      return *this;
    }

    inline Value &operator*() const {
      return **it;
    }

    inline Value &value() const {
      return **it;
    }

    inline SparseArrayIteratorTemplate &operator++() {
      ++*it;
      return *this;
    }

    inline SparseArrayIteratorTemplate operator++(int) {
      SparseArrayIteratorTemplate temp(*this);
      ++*this;
      return temp;
    }
    
    inline SparseArrayIteratorTemplate &operator--() {
      --*it;
      return *this;
    }

    inline SparseArrayIteratorTemplate operator--(int) {
      SparseArrayIteratorTemplate temp(*this);
      --*this;
      return temp;
    }

    inline node_t index() const {
      return it->index();
    }

    inline bool operator==(const SparseArrayIteratorTemplate &sai)
    {
      return *it==*sai.it;
    }
    
    inline bool operator!=(const SparseArrayIteratorTemplate &sai)
    {
      return !(*it==*sai.it);
    }

    inline void write(FILE *f) const
    {
      return it->write(f);
    }
  };

  class SparseArray {
   public:
    typedef SparseArrayIteratorTemplate<value_t> iterator;
    typedef SparseArrayIteratorTemplate<const value_t> const_iterator;

    virtual ~SparseArray() { }

    virtual SparseArray::iterator begin()=0;
    virtual SparseArray::const_iterator begin() const=0;

    virtual SparseArray::iterator end()=0;
    virtual SparseArray::const_iterator end() const=0;

    virtual SparseArray::iterator find(node_t index)=0;
    virtual SparseArray::const_iterator find(node_t index) const=0;

    virtual value_t operator[](node_t index) const;
    
    virtual node_t size() const=0;

    virtual void write(FILE *f) const=0;
  };
    
  value_t scal1(const SparseArray& sa1, const SparseArray& sa2);
  value_t scal2(const SparseArray& sa1, const SparseArray& sa2);

  value_t norm1(const SparseArray &a);
  value_t norm2(const SparseArray &a);
  
  value_t cos1(const SparseArray& sa1,const SparseArray& sa2);
  value_t cos2(const SparseArray& sa1,const SparseArray& sa2);
}

#endif /* SPARSE_ARRAY_H */
