/* $Id: TextVector2BinaryVector.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include "Vector.h"

using namespace std;
using namespace lsg;

int main(int argc, char **argv)
{
  if(argc!=3) {
    cerr << "Usage : " << argv[0] << " text_vector binary_vector" << endl;
    return EXIT_FAILURE;
  }

  cerr << "Loading text vector..." << endl;
  ifstream f(argv[1]);
  
  if(!f.is_open()) {
    cerr << "Loading error" << endl;
    return EXIT_FAILURE;
  }

  Vector v(f);

  cerr << "Storing binary vector..." << endl;
  if(!v.store(argv[2])) {
    cerr << "Storing error" << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
