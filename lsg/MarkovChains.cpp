/* $Id: MarkovChains.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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
#include <algorithm>
#include <iostream>

#include "Graph.h"
#include "MutableGraph.h"
#include "SparseArray.h"
#include "Vector.h"
#include "lsg.h"

using namespace std;

namespace {
  using namespace lsg;

  class DivideBy {
   public:
    DivideBy(value_t v) : d(v) { }
    inline void operator()(value_t &v) { v/=d; }

   private:
    value_t d;
  };

	value_t sqr(value_t x)
	{
		return x*x;
	}

	value_t l2dist(const RowVector& m1,const RowVector&m2,const RowVector&basem)
	{
		node_t size=m1.size();
		value_t s=0;
		for(node_t i=0;i<size;++i)s+=sqr(m1[i]-m2[i])/basem[i];
		return sqrt(s);
	}
}

namespace lsg {
  void stochastifyRows(Graph &g)
  {
    node_t n=g.getNbNodes();
    for(node_t i=0;i<n;++i) {
      value_t s=accumulate(g.row(i).begin(),g.row(i).end(),0.);
      if(s)
        for_each(g.row(i).begin(),g.row(i).end(),DivideBy(s));
    }
  }
  
  void stochastifyColumns(Graph &g)
  {
    node_t n=g.getNbNodes();
    for(node_t i=0;i<n;++i) {
      value_t s=accumulate(g.column(i).begin(),g.column(i).end(),0.);
      if(s)
        for_each(g.column(i).begin(),g.column(i).end(),DivideBy(s));
    }
  }

  void InvariantMeasure(const Graph &g, RowVector &v, unsigned niter,
                        bool verbose)
  {
    RowVector w(g.getNbNodes());

    for(unsigned i=0;i<niter;++i) {
      if(verbose)
        cerr << "Itération " << i << endl;

      w=v;
      v=v*g;
      
      if(verbose) {
        cerr << "Somme des éléments de v : " << v.sum() << endl;
        cerr << "Norme différence : " << abs(w-v).sum() << endl;
        cerr << "Dist différence : " << l2dist(w,v,v) << endl;
        cerr << "Max différence : " << abs(w-v).max() << endl;
        cerr << "Max : " << v.max() << endl;
        cerr << "Min : " << v.min() << endl;
        cerr << endl;
      }
    }
  }

  void anotherInvariantMeasure(const Graph &g, RowVector &v, unsigned niter,
                        bool verbose)
  {
		unsigned int size=g.getNbNodes();
    RowVector vo(v),voo(v),w(v),wo(v);
		RowVector a1(size),a2(size);
		value_t s1,s2,c;
		a1=v*g;
    for(unsigned n=0;n<(niter+2)/3;++n) {
      if(verbose)cerr << "Itération " << 3*n << endl;
			voo=a1;
			vo=voo*g;
			if(verbose){
        cerr << "Norme différence v : " << abs(vo-voo).sum() << endl;
        cerr << "Dist différence v : " << l2dist(voo,vo,vo) << endl;
				cerr << "Max différence v : " << abs(vo-voo).max() << endl;
        cerr << "Max : " << vo.max() << endl;
        cerr << "Min : " << vo.min() << endl;
				cerr<<endl;
				cerr << "Itération " << 3*n+1 << endl;
			}
      v=vo*g;
        cerr << "Norme différence v : " << abs(v-vo).sum() << endl;
        cerr << "Dist différence v : " << l2dist(vo,v,v) << endl;
        cerr << "Max différence v : " << abs(v-vo).max() << endl;
        cerr << "Max : " << v.max() << endl;
        cerr << "Min : " << v.min() << endl;
				cerr<<endl;
			for(node_t i=0;i<size;i++){
				a1[i]=vo[i]-voo[i];a2[i]=v[i]+voo[i]-2*vo[i];
			}
			s1=s2=c=0;
			for(node_t i=0;i<size;i++){
				s1+=a1[i]*a1[i]/v[i];s2+=a2[i]*a2[i]/v[i];
				c+=a1[i]*a2[i]/v[i];
			}
			s1=c/s2;
			for(node_t i=0;i<size;i++){
				w[i]=voo[i]-s1*a1[i];
			}
      
      if(verbose) {
        cerr << "Somme des éléments de w : " << w.sum() << endl;
        cerr << "Itération " << 3*n+2 << endl;
			a1=w*g;
        cerr << "Norme différence w : " << abs(w-a1).sum() << endl;
        cerr << "Dist différence w : " << l2dist(w,a1,v) << endl;
        cerr << "Max différence w : " << abs(w-a1).max() << endl;
        cerr << "Max : " << w.max() << endl;
        cerr << "Min : " << w.min() << endl;
				if(l2dist(w,a1,v)>l2dist(vo,v,v)){a1=v;cerr<<"Keep v"<<endl;}
        cerr << endl;
      }
    }
		v=a1;
  }

//  void anotherInvariantMeasure(const Graph &g, RowVector &v, unsigned niter,
//                        bool verbose)
//  {
//		unsigned int size=g.getNbNodes();
//    RowVector vo(v),voo(v),w(v),wo(v);
//		RowVector a1(size),a2(size);
//		value_t s1,s2,c;
//    for(unsigned i=0;i<niter;++i) {
//      if(verbose)
//        cerr << "Itération " << i << endl;
//
//			voo=vo;
//      vo=v;
//      v=v*g;
//			wo=w;
//			for(node_t i=0;i<size;i++){
//				a1[i]=vo[i]-voo[i];a2[i]=v[i]+voo[i]-2*vo[i];
//			}
//			s1=s2=c=0;
//			for(node_t i=0;i<size;i++){
//				s1+=a1[i]*a1[i]/w[i];s2+=a2[i]*a2[i]/w[i];
//				c+=a1[i]*a2[i]/w[i];
//			}
//			s1=c/s2;
//			for(node_t i=0;i<size;i++){
//				w[i]=voo[i]-s1*a1[i];
//			}
//      
//      if(verbose) {
//        cerr << "Somme des éléments de v : " << v.sum() << endl;
//        cerr << "Somme des éléments de w : " << w.sum() << endl;
//        cerr << "Norme différence v : " << abs(v-vo).sum() << endl;
//			a1=w*g;
//        cerr << "Norme différence w : " << abs(w-a1).sum() << endl;
//        cerr << "Max différence v : " << abs(v-vo).max() << endl;
//        cerr << "Max différence w : " << abs(w-a1).max() << endl;
//        cerr << "Max : " << w.max() << endl;
//        cerr << "Min : " << w.min() << endl;
//        cerr << endl;
//      }
//    }
//		if(abs(w-a1).sum()<abs(v-vo).sum())v=w;
//  }


  void Symmetrize(Graph &g, const Vector &measure)
		//Returns a graph with stationary measure measure
		//Assumes measure is an invariant probability measure for g
  {
    node_t size=g.getNbNodes();
    for(node_t i=0;i<size;i++){
      for(SparseArray::iterator it=g.row(i).begin(),
          itend=g.row(i).end();
          it!=itend;
          ++it){
        value_t p=.5*(*it+g(it.index(),i)*measure[it.index()]/measure[i]);
        *it=p;
        g(it.index(),i)=p*measure[i]/measure[it.index()];
      }
    }
  }

  void Reverse(Graph &g, const Vector &measure)
		//Returns a graph with invariant measure measure but opposite edges
		//Assumes measure is an invariant probability measure for g
  {
		g.transpose();
    node_t size=g.getNbNodes();
    for(node_t i=0;i<size;i++){
      for(SparseArray::iterator it=g.row(i).begin(),
          itend=g.row(i).end();
          it!=itend;
          ++it){
        *it=*it*measure[it.index()]/measure[i];
      }
    }
  }

}
