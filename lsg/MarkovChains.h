/* $Id: MarkovChains.h,v 1.2 2008-04-20 12:32:57 yann Exp $ */

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

#ifndef MARKOV_CHAINS_H
#define MARKOV_CHAINS_H

namespace lsg {
  class Graph;
	class MutableGraph;
  class RowVector;
  class Vector;

  void stochastifyRows(Graph &g);
  void stochastifyColumns(Graph &g);

  void InvariantMeasure(const Graph &g, RowVector &v, unsigned niter,
                        bool verbose);
		//Applies g niter times to v
  void anotherInvariantMeasure(const Graph &g, RowVector &v, unsigned niter,
                        bool verbose);

  void Symmetrize(Graph &g, const Vector &measure);
    //Returns a graph with stationary measure measure
    //Assumes measure is an invariant probability measure for g

  void Reverse(Graph &g, const Vector &measure);
    //Returns a graph with invariant measure measure but opposite edges
    //Assumes measure is an invariant probability measure for g

}

#endif /* MARKOV_CHAINS_H */
