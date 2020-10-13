/* $Id: SparseArray.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include <numeric>
#include <cmath>

#include "SparseArray.h"

using namespace std;

namespace lsg {
  value_t scal1(const SparseArray& sa1, const SparseArray& sa2)
  {
    value_t v=0.;

    SparseArray::const_iterator it1=sa1.begin(),
                                itend1=sa1.end(),
                                it2=sa2.begin(),
                                itend2=sa2.end();

    for(;it1!=itend1 && it2!=itend2;) {
      while(it1!=itend1 && *it1==0) ++it1;

      while(it2!=itend2 && *it2==0) ++it2;

      if(it1==itend1 || it2==itend2)
        break;

      if(it1.index()==it2.index()) {
        v+=sqrt(*it2 * *it1);
        ++it1,++it2;
      } else if(it1.index()<it2.index())
        ++it1;
      else
        ++it2;
    }

    return v;
  }

  template<typename Value> struct PlusAbs : public
    binary_function<Value,Value,Value> {
    inline Value operator()(const Value &v1,const Value &v2) const
    {
      return v1+abs(v2);
    }
  };

  value_t norm1(const SparseArray &sa)
  {
    return accumulate
      (sa.begin(),sa.end(),
       value_t(),
       PlusAbs<value_t>());
  }

  value_t cos1(const SparseArray& sa1,const SparseArray& sa2)
  {
    return scal1(sa1,sa2)/sqrt(norm1(sa1)*norm1(sa2));
  }

  value_t scal2(const SparseArray& sa1, const SparseArray& sa2)
  {
    value_t v=0.;

    SparseArray::const_iterator it1=sa1.begin(),
                                itend1=sa1.end(),
                                it2=sa2.begin(),
                                itend2=sa2.end();

    for(;it1!=itend1 && it2!=itend2;) {
      while(it1!=itend1 && *it1==0) ++it1;

      while(it2!=itend2 && *it2==0) ++it2;

      if(it1==itend1 || it2==itend2)
        break;

      if(it1.index()==it2.index()) {
        v+=*it2 * *it1;
        ++it1,++it2;
      } else if(it1.index()<it2.index())
        ++it1;
      else
        ++it2;
    }

    return v;
  }
  
  template<typename Value> struct PlusSquare : public
    binary_function<Value,Value,Value> {
    inline Value operator()(const Value &v1,const Value &v2) const
    {
      return v1+v2*v2;
    }
  };

  value_t norm2(const SparseArray &sa)
  {
    return accumulate
      (sa.begin(),sa.end(),
       value_t(),
       PlusSquare<value_t>());
  }

  value_t cos2(const SparseArray& sa1,const SparseArray& sa2)
  {
    return scal2(sa1,sa2)/sqrt(norm2(sa1)*norm2(sa2));
  }

  value_t SparseArray::operator[](node_t index) const
  {
    SparseArray::const_iterator it=find(index);

    if(it==end())
      return value_t();
    else
      return *it;
  }
}
