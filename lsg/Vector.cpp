/* $Id: Vector.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <cstring>

#include "Vector.h"
#include "Graph.h"
#include "SparseArray.h"

using namespace std;

namespace lsg {
  ostream &operator<<(ostream &o,const Vector &v)
  {
    streamsize oldPrecision=o.precision(20);

    unsigned size=v.size();
    o << size << "\n";
    for(unsigned i=0;i<size;++i) {
      o << v[i] << "\n";
    }
    o.precision(oldPrecision);

    return o;
  }

  ostream &Vector::dumpXml(ostream &o) const
  {
    streamsize oldPrecision=o.precision(20);

    unsigned size=this->size();

    o << "<?xml version='1.0' encoding='utf-8'?>\n";
    o << "<nodes>\n";
    
    for(unsigned i=0;i<size;++i)
      o << "  <node id='" << i << "' weight='" << (*this)[i] << "' />\n";

    o << "</nodes>\n";
    o.precision(oldPrecision);

    return o;
  }

  istream &operator>>(istream &in,Vector &v)
  {
    unsigned size;
    in >> size;
    v.resize(size);

    for(unsigned i=0;i<size;++i) {
      in>> v[i];
    }

    return in;
  }
    
  double Vector::average() const {
    return sum()/size();
  }
  
  double Vector::variance() const {
    Vector v(size());
    for(unsigned i=0;i<size();++i) {
      v[i]=(*this)[i]*(*this)[i];
    }
    double avg=average();
    return v.average()-avg*avg;
  }

  Vector::Vector(const Vector &v) : valarray<double>(v) { }

  Vector::Vector(std::istream &is)
  {
    is >> *this;
  }

  Vector::Vector(const std::string &filename)
  {
    FILE *f=fopen(filename.c_str(),"r");

    if(!f)
      return;
    
    char magic[4];
    if(fread(magic,1,4,f)!=4)
      return;
    if(strncmp(magic,"MSR0",4))
      return;

    unsigned int s;
    fread(&s,sizeof(unsigned int),1,f);

    resize(s);

    for(unsigned int i=0;i<s;++i)
      fread(&(*this)[i],sizeof(double),1,f);

    fclose(f);
  }

  bool Vector::store(const std::string &filename) const
  {
    FILE *f=fopen(filename.c_str(),"w");
    ftruncate(fileno(f),0);
    fseek(f,0,SEEK_SET);

    if(!f)
      return false;
    
    fwrite("MSR0",1,4,f);

    unsigned int s=size();
    fwrite(&s,sizeof(unsigned int),1,f);

    for(unsigned int i=0;i<s;++i)
      fwrite(&(*this)[i],sizeof(double),1,f);

    return !fclose(f);
  }

  Vector &Vector::operator=(const Vector &v) 
  {
    if(&v==this)
      return *this;
    
    valarray<double>::operator=(v);

    return *this;
  }

  const RowVector operator*(const RowVector &v, const Graph &g)
  {
    assert(g.getNbNodes()==v.size());
    
    RowVector res(v.size());
//    
//    for(unsigned i=0;i<v.size();++i) {
//      res[i]=0;
//      
//      for(SparseArray::const_iterator it=g.column(i).begin(),
//        itend=g.column(i).end();
//        it!=itend;
//        ++it) {
//        res[i]+=v[it.index()]* *it;
//      }
//    }

//    for(unsigned i=0;i<v.size();++i)res[i]=0;
//
		for(unsigned i=0;i<v.size();i++)if(v[i]){

			for(SparseArray::const_iterator it=g.row(i).begin(),
					itend=g.row(i).end();
					it!=itend;
					++it){
				res[it.index()]+=v[i]* *it;
			}
		}

    return res;
  }

  const ColumnVector operator*(const Graph &g,const ColumnVector &v)
  {
    assert(g.getNbNodes()==v.size());
    
    ColumnVector res(v.size());
    
//    for(unsigned i=0;i<v.size();++i) {
//      res[i]=0;
//      
//      for(SparseArray::const_iterator it=g.row(i).begin(),
//        itend=g.row(i).end();
//        it!=itend;
//        ++it) {
//        res[i]+=v[it.index()]* *it;
//      }
//    }

//    for(unsigned i=0;i<v.size();++i)res[i]=0;

		for(unsigned i=0;i<v.size();++i) if(v[i]){
      
      for(SparseArray::const_iterator it=g.column(i).begin(),
        itend=g.column(i).end();
        it!=itend;
        ++it) {
        res[it.index()]+=v[i]* *it;
      }
    }

    return res;
  }

  const Vector operator*(double d,const Vector &v)
  {
    return Vector(d*static_cast<valarray<double> >(v));
  }

  const Vector operator*(const Vector &v,double d)
  {
    return Vector(d*static_cast<valarray<double> >(v));
  }

  const Vector operator+(const Vector &v1,const Vector &v2)
  {
    return Vector(static_cast<valarray<double> >(v1)+
                  static_cast<valarray<double> >(v2));
  }
}
