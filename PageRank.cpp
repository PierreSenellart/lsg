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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include <utility>

#include "PackedGraph.h"
#include "Vector.h"

using namespace std;
using namespace lsg;

const double damping_factor=1.-0.15;
const double threshold=.01;

struct Comparator {
  bool operator()(const pair<node_t,double> &a,const pair<node_t,double> &b) const {
    return a.second>b.second?1:(a.second<b.second?0:(a.first<b.first?1:0));
  }
};

int main(int argc, char **argv)
{
  if(argc!=3) {
    cerr << "Usage: " << argv[0] << " graph out";
    return EXIT_FAILURE;
  }

  cerr << "Loading graph..." << endl;
  PackedGraph g(argv[1]);

  node_t size=g.getNbNodes();

  RowVector uniform(size);

  for(node_t i=0;i<size;++i)
    uniform[i]=1./size;

  RowVector v=uniform,w=v;

  RowVector uniform2=(1.-damping_factor)*uniform;

  {double difference;
  unsigned i=0;
  do {
    cerr << "Itération " << i+1 << endl;

    w=v;
    v=damping_factor*v*g+uniform2;

    cerr << "Somme des éléments de v : " << v.sum() << endl;
    cerr << "Norme différence : " << abs(w-v).sum() << endl;
    cerr << "Max différence : " << abs(w-v).max() << endl;
    cerr << "Max : " << v.max() << endl;
    cerr << "Min : " << v.min() << endl;

    difference=(abs(w-v)/v).max();
    cerr << "Différence relative : " << (abs(w-v)/v).max() << endl;
    cerr << endl;
    ++i;
  } while(difference>=threshold);}

  set<pair<node_t,double>,Comparator> s;
  for(node_t i=0;i<size;++i)
    s.insert(make_pair(i,v[i]));

  ofstream out(argv[2]);
  out << setprecision(10);
  out<< scientific;
  node_t i=0;
  for(set<pair<node_t,double> >::const_iterator it=s.begin(),
                                              itend=s.end();
      it!=itend;
      ++it,++i) {
    out << g.getLabel(it->first) << "\t" << it->second << "\t" << i+1 << "\n";
  }

  return EXIT_SUCCESS;
}
