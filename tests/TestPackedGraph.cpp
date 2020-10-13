/* $Id: TestPackedGraph.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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
#include <vector>
#include <sstream>

#include "MutableGraph.h"
#include "PackedGraph.h"
#include "TempFile.h"

using namespace lsg;

namespace tut {
  struct TestPackedGraphData { 
    static const std::string edge_list_example;
    static const std::string edge_list_with_values_example;
  };

  const std::string TestPackedGraphData::edge_list_example=
    "4\n"
    "no values\n"
    "0 1 2\n"
    "1 0\n"
    "2 1 3\n";
  
  const std::string TestPackedGraphData::edge_list_with_values_example=
    "4\n"
    "with values\n"
    "0 1,1 2,0.5\n"
    "1 0,0.25\n"
    "2 1,400 3,123\n"
    "3\n";

  typedef test_group<TestPackedGraphData> testgroup;
  typedef testgroup::object testobject;
  testgroup packedgraph_testgroup("PackedGraph");

  // MutableGraph constuction from edge list with values
  // then storage in a PackedGraph, then loaded, then dumped
  // should be a no-op
  template<> template<>
    void testobject::test<1>()
  {
    std::istringstream iss(edge_list_with_values_example);

    MutableGraph g(iss);
    TempFile f;

    g.store(f.name());

    PackedGraph h(f.name());

    ensure_equals("h==g",h,g);

    std::ostringstream oss;

    oss << h;

    ensure_equals("edge list with values",oss.str(),
                                          edge_list_with_values_example);
  }

  // Values can be modified, and persist in the file.
  template<> template<>
    void testobject::test<2>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);
    TempFile f;

    g.store(f.name());

    {
      PackedGraph h(f.name());

      ensure_equals("h==g",h,g);
      // Values can be modified
      h(0,1)=2.;
      ensure("h!=g",h!=g);

      try {
        // But non-existent values cannot be added
        h(2,0)=2.;
        fail("non-existent values cannot be added");
      } catch(const std::domain_error &e) { }
    }

    // Let's reopen it
    PackedGraph h(f.name());

    ensure_equals("h(0,1)",h(0,1),2.);
    ensure_equals("h(0,2)",h(0,2),1.);
    
    // read access to non-existant h(0,3) should work for h non-constant
    // thanks to proxy class
    ensure_equals("h(0,3)",h(0,3),0.);
  }

  // storeWithTransposedEdges
  template<> template<>
    void testobject::test<3>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);
    TempFile f;

    g.storeWithTransposedEdges(f.name());

    PackedGraph h(f.name());

    ensure_equals("h==g",h,g);
    // Values can be modified
    h(0,1)=2.;
    ensure("h!=g",h!=g);

    try {
      // Non-existent values cannot be added
      h(0,3)=2.;
      fail("non-existent values cannot be added");
    } catch(const std::domain_error &e) { }

    // But transposed edges can be modified
    ensure_equals("h(2,0)",h(2,0),0.);
    h(2,0)=2.;
    ensure_equals("h(2,0)",h(2,0),2.);
  }

  // Transposition
  template<> template<>
    void testobject::test<4>()
  {
    std::istringstream iss(edge_list_with_values_example);

    MutableGraph g(iss);
    TempFile f;

    g.store(f.name());

    {
      PackedGraph h(f.name());

      h.transpose();
    
      ensure_equals("h(0,1)",h(0,1),.25);
      ensure_equals("h(1,0)",h(1,0),1.);
      ensure_equals("h(2,0)",h(2,0),.5);
      ensure_equals("h(0,2)",h(0,2),0);
    }

    PackedGraph h2(f.name());

    ensure_equals("h2(0,1)",h2(0,1),.25);
    ensure_equals("h2(1,0)",h2(1,0),1.);
    ensure_equals("h2(2,0)",h2(2,0),.5);
    ensure_equals("h2(0,2)",h2(0,2),0);
  }
  
  // Subgraph construction
  template<> template<>
    void testobject::test<5>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);
    TempFile f;

    std::vector<bool> vec(4);
    vec[0]=vec[2]=vec[3]=true;
    vec[1]=false;

    g.storeSubgraph(f.name(),vec);

    PackedGraph h(f.name());

    ensure_equals("h==MutableGraph(g,vec)",h,MutableGraph(g,vec));
  }
}
