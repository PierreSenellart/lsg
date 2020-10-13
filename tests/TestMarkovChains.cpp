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

#include "Vector.h"
#include "MarkovChains.h"
#include "MutableGraph.h"
#include "PackedGraph.h"
#include "TempFile.h"
#include "ConnectedComponents.h"

using namespace lsg;

namespace tut {
  struct TestMarkovChainsData { 
    static const std::string edge_list_example;
  };

  const std::string TestMarkovChainsData::edge_list_example=
    "4\n"
    "no values\n"
    "0 1 2\n"
    "1 0\n"
    "2 1 3\n"
    "3 1\n";
  
  typedef test_group<TestMarkovChainsData> testgroup;
  typedef testgroup::object testobject;
  testgroup markovchains_testgroup("MarkovChains");

  // Symmetrization
  template<> template<>
    void testobject::test<1>()
  {
    MutableGraph g=RandomGraph(300,.25);
    node_t size=g.getNbNodes();
		
    std::vector<node_t> comp;
    stronglyConnectedComponents(g,comp);
    std::vector<bool> firstscc(size,false);
    for(node_t i=0;i<size;i++)
      if(comp[i]==comp[0])
        firstscc[i]=true;

    MutableGraph scc(g,firstscc);
    size=scc.getNbNodes();
    if(size==1)return ;

    MutableGraph gcopy(scc);
    stochastifyRows(scc);

    RowVector v(size);
   
    for(node_t i=0;i<size;++i)
      v[i]=1./size;

    InvariantMeasure(scc,v,100,false);

    TempFile f;

    scc.storeWithTransposedEdges(f.name());

    PackedGraph h(f.name());

    Symmetrize(h,v);
    Symmetrize(scc,v);
    RowVector w=v*h;
    RowVector sccw=v*scc;

    ensure("diff(scc)<1e-5",abs(sccw-v).sum()<1e-5);
    ensure("diff(h)<1e-5",abs(w-v).sum()<1e-5);
  }
}
