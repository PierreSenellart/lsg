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

#include <functional>
#include <algorithm>
#include <string>
#include <sstream>

#include "MutableGraph.h"
#include "ConnectedComponents.h"
#include "TempFile.h"

using namespace lsg;

namespace tut {
  struct TestConnectedComponentsData { 
    static const std::string edge_list_example;
  };

  const std::string TestConnectedComponentsData::edge_list_example=
    "6\n"
    "no values\n"
    "0 1\n"
    "1 0 2\n"
    "2 0 3\n"
    "4 5\n";
  
  typedef test_group<TestConnectedComponentsData> testgroup;
  typedef testgroup::object testobject;
  testgroup connectedcomponents_testgroup("ConnectedComponents");

  // Weakly connected components
  template<> template<>
    void testobject::test<1>()
  {
    std::istringstream iss(edge_list_example);

    MutableGraph g(iss);

    std::vector<node_t> comp;

    weaklyConnectedComponents(g,comp);

    ensure_equals("all nodes have a component",
                  *min_element(comp.begin(),comp.end()),1u);
    ensure_equals("nb of connected components",
                  *max_element(comp.begin(),comp.end()),2u);
    ensure("component 1",comp[0]==comp[1] && comp[1]==comp[2] 
                                          && comp[2]==comp[3]);
    ensure("component 2",comp[4]==comp[5]);
  }
  
  // Strongly connected components
  template<> template<>
    void testobject::test<2>()
  {
    std::istringstream iss(edge_list_example);

    const MutableGraph g(iss);

    std::vector<node_t> comp;

    stronglyConnectedComponents(g,comp);

    ensure_equals("all nodes have a component",
                  *min_element(comp.begin(),comp.end()),1u);
    ensure_equals("nb of connected components",
                  *max_element(comp.begin(),comp.end()),4u);
    ensure("component 1",comp[0]==comp[1] && comp[1]==comp[2]);

    MutableGraph contracted;
    contractedGraph(g,comp,contracted);

    ensure_equals("size of contracted",contracted.getNbNodes(),4u);
    ensure("component 1 -> component 2",contracted(comp[0]-1,comp[3]-1));
    ensure("not(component 2 -> component 1)",!contracted(comp[3]-1,comp[0]-1));

    std::vector<bool> first_scc(g.getNbNodes());
    transform(comp.begin(),comp.end(),
              first_scc.begin(),
              bind2nd(std::equal_to<node_t>(),comp[0]));

    MutableGraph restriction(g,first_scc);
    
    ensure_equals("size of restriction",restriction.getNbNodes(),3u);
    ensure("restriction(0,1)",restriction(0,1));
    ensure("restriction(1,0)",restriction(1,0));
    ensure("restriction(1,2)",restriction(1,2));
    ensure("restriction(2,0)",restriction(2,0));
    ensure("!restriction(0,2)",!restriction(0,2));
    ensure("!restriction(2,1)",!restriction(2,1));
  }
}
