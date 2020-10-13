/* $Id: Vector.h,v 1.2 2008-04-20 12:32:57 yann Exp $ */

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

#ifndef VECTOR_H
#define VECTOR_H

#include <valarray>
#include <iosfwd>

namespace lsg {
  class Graph;

  class Vector: public std::valarray<double>
  {
  public:
    Vector(unsigned int s=0) : std::valarray<double>(s) {}
    Vector(const Vector &v);
    Vector(const std::valarray<double> d) : std::valarray<double>(d) {}
    Vector(std::istream &is);
    Vector(const std::string &filename);
    double average() const;
    double variance() const;
    Vector &operator=(const Vector &v);
    bool store(const std::string &filename) const;
    std::ostream &dumpXml(std::ostream &o) const;
  };

  std::ostream &operator<<(std::ostream &o,const Vector &v);
  std::istream &operator>>(std::istream &i,Vector &v);

  const Vector operator*(double d,const Vector &v);
  const Vector operator*(const Vector &v,double d);
  const Vector operator+(const Vector &v1,const Vector &v2);

  class RowVector : public Vector
  {
  public:
    RowVector(unsigned int s=0) : Vector(s) {}
    RowVector(const Vector &v) : Vector(v) {}
    RowVector(std::istream &is) : Vector (is) {}
    RowVector(const std::string &filename): Vector (filename) {}

    RowVector &operator=(const Vector &v)
      { Vector::operator=(v); return *this; }

    friend const RowVector operator*(const RowVector &v,const Graph &g);
  };
    
  const RowVector operator*(const RowVector &v,const Graph &g);
  
  class ColumnVector: public Vector
  {
  public:
    ColumnVector(unsigned int s=0) : Vector(s) {}
    ColumnVector(const Vector &v) : Vector (v) {}
    ColumnVector(std::istream &is) : Vector (is) {}
    ColumnVector(const std::string &filename): Vector (filename) {}
    
    inline ColumnVector &operator=(const Vector &v)
      { Vector::operator=(v); return *this; }
    
    friend const ColumnVector operator*(const Graph &g,const ColumnVector &v);
  };
    
  const ColumnVector operator*(const Graph &g,const ColumnVector &v);
}

#endif /* VECTOR_H */
