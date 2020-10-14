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

#include "PackedGraph.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace std;
using namespace lsg;

int main(int argc, char **argv) {
  if(argc<3) {
    cerr << "Usage: " << argv[0] << " graph node_label ..." << endl;
    return EXIT_FAILURE;
  }

  PackedGraph g(argv[1]);

  for(int i=0; i<20; ++i)
    cerr << i << " " << g.getLabelSize(i) << " " << g.getLabel(i) << endl;

  for(auto i = 2; i<argc; ++i) {
    auto node = g.getNodeWithLabel(argv[i]);

    cerr << argv[i] << endl;

    if(node == static_cast<node_t>(-1)) {
      cerr << "Not found" << endl;;
    } else {
      cerr << "In degree: " << setw(10) << g.inDegree(node) << endl;
      cerr << "Out degree: " << setw(10) << g.outDegree(node) << endl;
    }

    cerr << endl;
  }

  return EXIT_SUCCESS;
}
