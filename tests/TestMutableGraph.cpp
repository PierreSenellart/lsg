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

using namespace lsg;

namespace tut {
  struct TestMutableGraphData { 
    static const std::string edge_list_example;
    static const std::string edge_list_with_values_example;
  };

  const std::string TestMutableGraphData::edge_list_example=
    "4\n"
    "no values\n"
    "0 1 2\n"
    "1 0\n"
    "2 1 3\n";
  
  const std::string TestMutableGraphData::edge_list_with_values_example=
    "4\n"
    "with values\n"
    "0 1,1 2,0.5\n"
    "1 0,0.25\n"
    "2 1,400 3,123\n"
    "3\n";


  typedef test_group<TestMutableGraphData> testgroup;
  typedef testgroup::object testobject;
  testgroup mutablegraph_testgroup("MutableGraph");

  // MutableGraph default construction
  template<> template<>
    void testobject::test<1>()
  {
    const MutableGraph g;
    ensure("nb nodes",g.getNbNodes()==0);
    ensure("nb edges",g.getNbEdges()==0);
  }

  // MutableGraph constuction from edge list
  // Various modifications
  template<> template<>
    void testobject::test<2>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);

    g(1,0)=-7;     // Let's modify an edge
    g.remove(2,1); // Let's remove an edge
    g(3,3)=35;     // Let's insert a new edge

    ensure("nb nodes",g.getNbNodes()==4);
    ensure("nb edges",g.getNbEdges()==5);
    ensure_equals("g(0,1)",g(0,1),1.);
    ensure_equals("g(1,0)",g(1,0),-7.);
    ensure_equals("g(2,1)",g(2,1),0.);
    ensure_equals("g(2,3)",g(2,3),1.);
    ensure_equals("g(3,2)",g(3,2),0.);
    ensure_equals("g(3,3)",g(3,3),35.);
  }
  
  // MutableGraph constuction from edge list with values
  // (with format chosen appropriately)
  // then dump should be a no-op
  template<> template<>
    void testobject::test<3>()
  {
    std::istringstream iss(edge_list_with_values_example);

    const MutableGraph g(iss);

    std::ostringstream oss;

    oss << g;

    ensure_equals("edge list with values",oss.str(),
                                          edge_list_with_values_example);
  }

  // MutableGraph copy
  template<> template<>
    void testobject::test<4>()
  {
    std::istringstream iss(edge_list_with_values_example);

    const MutableGraph g(iss);

    MutableGraph h1(g);

    ensure_equals("g==h1 (constr)",h1,g);

    MutableGraph h2;
    h2=g;

    ensure_equals("g==h2 (copy)",h2,g);

    h2(0,3)=1;
    ensure("g!=h2 (copy)",h2!=g);
  }
}
