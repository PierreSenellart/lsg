/* $Id: NodeArray.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include <stdexcept>

#include "NodeArray.h"
#include "Graph.h"

using namespace std;

namespace lsg {
  value_t NodeArray::operator[](node_t index) const
  {
    map<node_t,value_t>::const_iterator it=m.find(index);

    if(it==m.end())
      return value_t();
    else
      return it->second;
  }
}

namespace {
  using namespace lsg;

  void propagate(const Graph &g, NodeArray &na, bool backward)
  {
    NodeArray result;

    for(SparseArray::iterator it=na.begin(),
                           itend=na.end();
        it!=itend;
        ++it) {
      const SparseArray &gsa=backward?g.column(it.index()):g.row(it.index());
      
      for(SparseArray::const_iterator git=gsa.begin(),
                                   gitend=gsa.end();
          git!=gitend;
          ++git) {
        result.insert(git.index(),1.);
      }
    }

    na=result;
  }
}

namespace lsg {
  void directedSphere
    (const Graph &g, node_t node, const std::string &direction,
     NodeArray &result)
  {
    if(direction.find_first_not_of("FB")!=string::npos)
      throw std::invalid_argument("Incorrect direction string");

    result.clear();
    
    result.insert(node,1.);

    std::string::size_type size=direction.size();
    for(std::string::size_type i=0;i<size;++i)
      propagate(g,result,direction[i]=='B'?true:false);
  }

	void addNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result)
	{
		NodeArray tmp;
		NodeArray &res2=(&result==&a1||&result==&a2)?tmp:result;
		res2.clear();
		for(SparseArray::const_iterator it1=a1.begin(),it2=a2.begin(),it1end=a1.end(),it2end=a2.end();
				it1!=it1end||it2!=it2end; ){
			while(it1!=it1end&&*it1==0)++it1;
			while(it2!=it2end&&*it2==0)++it2;
			if(it1==it1end&&it2==it2end)break;
			if(it1!=it1end&&(it2==it2end||it1.index()<it2.index())){
				res2.insert(it1.index(),it1.value());
				++it1;
			}else if(it2!=it2end&&(it1==it1end||it2.index()<it1.index())){
				res2.insert(it2.index(),it2.value());
				++it2;
			}
			else{ //it1.index()==it2.index()
				res2.insert(it1.index(),it1.value()+it2.value());
				++it1;++it2;
			}
		}
		if(&result==&a1||&result==&a2)result=tmp;
		//		for(it=a1.begin,itend=a1.end();it!=itend;++it)
		//			result.append(it.index(),it.value());
		//		for(it=a2.begin,itend=a2.end();it!=itend;++it)
		//			result.m[it.index()]+=it.value();
	}
	
	void multNodeArray(const NodeArray& a1,const NodeArray& a2,NodeArray& result)
	{
		NodeArray tmp;
		NodeArray &res2=(&result==&a1||&result==&a2)?tmp:result;
		res2.clear();
		for(SparseArray::const_iterator it1=a1.begin(),it2=a2.begin(),it1end=a1.end(),it2end=a2.end();
				it1!=it1end&&it2!=it2end; ){
			while(it1!=it1end && *it1==0)++it1;
			while(it2!=it2end && *it2==0)++it2;
			if(it1==it1end||it2==it2end)break;
			if(it1.index()==it2.index())res2.insert(it1.index(),it1.value()*it2.value());
			else if(it1.index()<it2.index())++it1;
			else ++it2;
		}
		if(&result==&a1||&result==&a2)result=tmp;
	}
}
