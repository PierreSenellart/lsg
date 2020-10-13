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
#include <fstream>
#include <sstream>

#include "Vector.h"
#include "PackedGraph.h"
#include "MarkovChains.h"

using namespace std;
using namespace lsg;

int main(int argc, char **argv)
{
  if(argc!=4&&argc!=5) {
    cerr << "Usage : " << argv[0] << " graph niter measure" << endl;
    cerr << "   or : " << argv[0] << " graph niter measure startmeasure" << endl;
    return EXIT_FAILURE;
  }

  cerr << "Loading graph..." << endl;
  PackedGraph g(argv[1]);

  std::stringstream ss(argv[2]);
  unsigned int niter;
  ss >> niter;

  cerr << "Computing invariant measure..." << endl;

  node_t size=g.getNbNodes();

  RowVector v(size);

  if(argc==4)for(node_t i=0;i<size;++i)
    v[i]=1./size;
	else if(argc==5){
		v=RowVector(argv[4]);
	}

//  anotherInvariantMeasure(g,v,niter,true);
	InvariantMeasure(g,v,niter,true);
  
  cerr << "Storing measure..." << endl;
	v.store(argv[3]);

  return EXIT_SUCCESS;
}
