/* $Id: TestNodeArray.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include "tut/tut.h"

#include <string>
#include <sstream>

#include "MutableGraph.h"
#include "NodeArray.h"

using namespace lsg;

namespace tut {
  struct TestNodeArrayData { 
    static const std::string edge_list_example;
  };
  
  const std::string TestNodeArrayData::edge_list_example=
    "4\n"
    "no values\n"
    "0 1 2\n"
    "1 0\n"
    "2 1 3\n";

  typedef test_group<TestNodeArrayData> testgroup;
  typedef testgroup::object testobject;
  testgroup nodearray_testgroup("NodeArray");

  // DirectedSphere test
  template<> template<>
    void testobject::test<1>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);

    NodeArray sphere;

    directedSphere(g,0,"",sphere);
    ensure_equals("  : sphere[0]",sphere[0],1);
    ensure_equals("  : sphere[1]",sphere[1],0);
    ensure_equals("  : sphere[2]",sphere[2],0);
    ensure_equals("  : sphere[3]",sphere[3],0);
    
    directedSphere(g,0,"F",sphere);
    ensure_equals(" F: sphere[0]",sphere[0],0);
    ensure_equals(" F: sphere[1]",sphere[1],1);
    ensure_equals(" F: sphere[2]",sphere[2],1);
    ensure_equals(" F: sphere[3]",sphere[3],0);

    directedSphere(g,0,"B",sphere);
    ensure_equals(" B: sphere[0]",sphere[0],0);
    ensure_equals(" B: sphere[1]",sphere[1],1);
    ensure_equals(" B: sphere[2]",sphere[2],0);
    ensure_equals(" B: sphere[3]",sphere[3],0);

    directedSphere(g,0,"BF",sphere);
    ensure_equals("BF: sphere[0]",sphere[0],1);
    ensure_equals("BF: sphere[1]",sphere[1],0);
    ensure_equals("BF: sphere[2]",sphere[2],0);
    ensure_equals("BF: sphere[3]",sphere[3],0);

    directedSphere(g,0,"FF",sphere);
    ensure_equals("FB: sphere[0]",sphere[0],1);
    ensure_equals("FB: sphere[1]",sphere[1],1);
    ensure_equals("FB: sphere[2]",sphere[2],0);
    ensure_equals("FB: sphere[3]",sphere[3],1);
  }
}
