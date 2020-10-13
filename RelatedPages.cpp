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
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include<set>
#include<map>

#include "PackedGraph.h"
#include "ConnectedComponents.h"
#include "SparseArray.h"
#include "MarkovChains.h"
#include "Vector.h"
#include "NodeArray.h"
#include "TempFile.h"

using namespace std;
using namespace lsg;

void PrintLinksFrom(const Graph& g,node_t node)
{
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;++j){
	 cerr<<g.getLabel(j.index())<<" "<<j.value()<<endl;
 }
}

void PrintLinksTo(const Graph& g,node_t node)
{
 for(SparseArray::const_iterator j=g.column(node).begin(),jend=g.column(node).end();
		 j!=jend;++j){
	 cerr<<g.getLabel(j.index())<<" "<<j.value()<<endl;
 }
}

struct Comparator {
  inline bool operator()(pair<node_t,value_t> a,pair<node_t,value_t> b) {
    return a.second>b.second?true:(a.second<b.second?false:a.first<b.first);
  }
};

bool IsLinked(const Graph&g,node_t source,node_t dest)
{
 return g(source,dest)>0.;
}

template<class T>
void addResultsToFileForSQLImport(const string &filename,
                                  const string &method,
                                  node_t base_article,
                                  const Graph &g,
                                  const T&v,
                                  unsigned nb);

template<class T>
void PrintNamesOfBest(const Graph &g,const T& v, node_t nbest,const std::string &method, node_t base_article)
{
  vector<pair<node_t,value_t> > s(v.size());
  for(node_t i=0;i<v.size();++i){
    s[i]=make_pair(i,v[i]);
  }

  partial_sort(s.begin(),s.begin()+nbest,s.end(),Comparator());

  vector<pair<node_t,value_t> >::const_iterator
    it=s.begin(),itend=s.end();

  if(nbest==0){
    node_t i;
    for(i=0;it!=itend&&it->second>0;++i,++it){
      cout<<g.getLabel(it->first)<<" "<<it->second<<endl;
    }
    cout<<"These were the "<<i<<" best ones"<<endl;
  }else{
    for(node_t i=0;i<nbest&&it!=itend&&it->second>0.;++i,++it){
      cout<<(IsLinked(g,base_article,it->first)?"*":"")<<g.getLabel(it->first)<<" "<<it->second<<endl;
    }
  }
 
  if(!method.empty())
    addResultsToFileForSQLImport("../evaluation",method,base_article,g,v,30);
}

template<class T>
void addResultsToFileForSQLImport(const string &filename,
                                  const string &method,
                                  node_t base_article,
                                  const Graph &g,
                                  const T&v,
                                  unsigned nb)
{
  ofstream out(filename.c_str(),ios::app);
 
  vector<pair<node_t,value_t> > s(v.size());
  for(node_t i=0;i<v.size();++i){
    s[i]=make_pair(i,v[i]);
  }

  partial_sort(s.begin(),s.begin()+nb,s.end(),Comparator());

  vector<pair<node_t,value_t> >::const_iterator
    it=s.begin(),itend=s.end();

  std::string word=g.getLabel(base_article);
  for(node_t i=0;i<nb&&it!=itend&&it->second>0.;++i,++it){
    out << "\"" << word << "\";";
    out << "\"" << method << "\";";
    out << i << ";";
    out << "\"" << g.getLabel(it->first) << "\"" << "\n";
  }
}

// void PrintNamesOfBest(const ColumnVector& v, value_t threshold)
// {
//  set<pair<node_t,value_t>,Comparator> s;
//  for(node_t i=0;i<v.size();++i){
// 	 s.insert(make_pair(i,v[i]));
//  }
//  set<pair<node_t,value_t>,Comparator>::const_iterator it=s.begin(),itend=s.end();
//  value_t v0=v.variance(),vcum=0,m=v.average();
//  cout<<v0<<" "<<m<<endl;
//  node_t i;
//  for(i=0;it!=itend&&vcum<threshold*v0;++i,++it){
// 	 cout<<nameof(it->first)<<" "<<it->second<<endl;
// 	 vcum+=(it->second-m)*(it->second-m)/v.size();
//  }
//  cout<<vcum<<endl;
//  cout<<"These were the "<<i<<" best ones"<<endl;
// }

unsigned int NForwardCocitation(const Graph&g,unsigned i,unsigned j)//Number of common links between nodes i and j
{
  static node_t previousi=0;static bool flag=false;
  static vector<bool> v;//indicates which nodes are pointed to by previousi
  if(i!=previousi||!flag){
    flag=true;previousi=i;
    v=vector<bool> (g.getNbNodes(),0);
    for(SparseArray::const_iterator it=g.row(i).begin(),itend=g.row(i).end();
        it!=itend;++it){
      v[it.index()]=true;
    }
  }
  unsigned int c=0;
  for(SparseArray::const_iterator it=g.row(j).begin(),itend=g.row(j).end();it!=itend;++it)
    if(v[it.index()])c++;
  return c;
}

unsigned int NBackwardCocitation(const Graph&g,unsigned i,unsigned j)//Number of nodes pointing to both i and j
{
  static node_t previousi=0;static bool flag=false;
  static vector<bool> v;//indicates which nodes are pointed to by previousi
  if(i!=previousi||!flag){
    flag=true;previousi=i;
    v=vector<bool> (g.getNbNodes(),false);
    for(SparseArray::const_iterator it=g.column(i).begin(),itend=g.column(i).end();
        it!=itend;++it){
      v[it.index()]=true;
    }
  }
  unsigned int c=0;
  for(SparseArray::const_iterator it=g.column(j).begin(),itend=g.column(j).end();it!=itend;++it)
    if(v[it.index()])c++;
  return c;
}

void ForwardPagerank(const Graph& g,node_t node,Vector&v,const RowVector&eq)
{
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;
		 ++j){
	 v[j.index()]=eq[j.index()];
 }
 return;
}

void BackPagerank(const Graph& g,node_t node,Vector&v,const RowVector&eq)
{
 for(SparseArray::const_iterator j=g.column(node).begin(),jend=g.column(node).end();
		 j!=jend;
		 ++j){
	 v[j.index()]=eq[j.index()];
 }
 return;
}

void nodearray1(const Graph&g,node_t node,NodeArray& a)
{
 a.clear();
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;++j){
	 a.insert(j.index(),1);
 }
}

void nodearraytf(const Graph&g,node_t node,NodeArray& a)
{
 a.clear();
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;++j){
	 a.insert(j.index(),j.value());
 }
}

void nodearraytfidf(const Graph&g,node_t node,NodeArray& a)
{
 a.clear();
 node_t size=g.getNbNodes();
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;++j){
	 a.insert(j.index(),j.value()*log((1.*size)/g.inDegree(j.index())));
 }
}

void nodearraytfinfo(const Graph&g,node_t node,NodeArray& a,const RowVector& eqm)
{
 a.clear();
 for(SparseArray::const_iterator j=g.row(node).begin(),jend=g.row(node).end();
		 j!=jend;++j){
	 a.insert(j.index(),j.value()*log(1./eqm[j.index()]));
 }
}

void SomeNeighborhood(const Graph&g, node_t node, NodeArray& a)
{
 NodeArray n;
 directedSphere(g,node,"",a);
 directedSphere(g,node,"F",n);
 addNodeArray(a,n,a);
 directedSphere(g,node,"FB",n);
 addNodeArray(a,n,a);
 directedSphere(g,node,"B",n);
 addNodeArray(a,n,a);
 directedSphere(g,node,"BF",n);
 addNodeArray(a,n,a);
}

void PageRankOfLinks(const Graph&g, node_t node, const RowVector& v)
{
 RowVector v2(v.size());
 for(SparseArray::const_iterator i=g.row(node).begin(),iend=g.row(node).end();
		 i!=iend;++i)v2[i.index()]=v[i.index()];
 v2[node]=v[node];
 PrintNamesOfBest(g,v2,30u,"PageRankOfLinks",node);
}

void NeighborhoodPageRank(const Graph&g, node_t node, const RowVector& eq)
{
 NodeArray n;
 cerr<<"Computing desired neighborhood"<<endl;
 SomeNeighborhood(g,node,n);
 unsigned int size=g.getNbNodes();
 cerr<<n.size()<<endl;
 cerr<<"Extracting subgraph"<<endl;
 vector<bool> whichnodes(size,false);
 for(SparseArray::iterator i=n.begin(),iend=n.end();
		 i!=iend;++i)if(i.value())whichnodes[i.index()]=true;
 TempFile f;
 g.storeSubgraph(f.name(),whichnodes);
   RowVector v2(size);
   for(node_t i=0,c=0;i<size;++i)if(whichnodes[i])v2[c++]=eq[i];
 PackedGraph ng(f.name());
 node_t nnode=ng.getNodeWithLabel(g.getLabel(node));
 if(nnode==(node_t)(-1))cerr<<"This should not happen"<<endl;
 unsigned int nsize=ng.getNbNodes();
 cerr<<ng.getNbNodes()<<endl;
 cerr<<"Computing its strongly connected components"<<endl;
 vector<node_t> comp(nsize);
 stronglyConnectedComponents(ng,comp);
 cerr<<"Extracting subsubgraph"<<endl;
 node_t compnum=comp[nnode];
 whichnodes=vector<bool> (nsize,false);
 for(node_t i=0;i<nsize;++i)if(comp[i]==compnum)whichnodes[i]=true;
 MutableGraph ngc(ng,whichnodes);
 ng.destroy();
   for(node_t i=0,c=0;i<nsize;++i)if(whichnodes[i])v2[c++]=v2[i];
 nsize=ngc.getNbNodes();cerr<<nsize<<endl;
 cerr<<"Stochastifying rows"<<endl;
 stochastifyRows(ngc);
 cerr<<"Computing invariant measure"<<endl;
 RowVector v(nsize);for(node_t i=0;i<nsize;++i)v[i]=1./nsize;
 for(int t=0;t<10;t++){
	 v=v*ngc;
	 cerr<<endl<<t<<" "<<endl;
//	 PrintNamesOfBest(ngc,v,30u);
 }

 cerr<<endl;
 //RowVector v2(nsize);
// for(node_t i=0;i<nsize;++i){
//	 v2[i]=eq[g.getNodeWithLabel(ngc.getLabel(i))];
//	 if(!(i%1000))cerr<<i<<endl;
// }
 //value_t s=0;for(node_t i=0;i<nsize;++i)s+=v2[i];
 //for(node_t i=0;i<nsize;++i)v[i]-=v2[i]/s;
 PrintNamesOfBest(ngc,v,30u,"NeighborhoodPageRank",
                  ngc.getNodeWithLabel(g.getLabel(node)));
}

void NCocitations(const Graph& g, node_t node)
{
 NodeArray n;directedSphere(g,node,"BF",n);
 vector<unsigned int> cocit(g.getNbNodes(),0);
 for(SparseArray::iterator i=n.begin(),iend=n.end();
		 i!=iend;++i){
	 cocit[i.index()]=NBackwardCocitation(g,node,i.index());
	 //cocit[j]=NForwardCocitation(g,node,j);
	 //cocit[j]=NForwardCocitation(g,node,j)+NBackwardCocitation(g,node,j);
 }
 PrintNamesOfBest(g,cocit,30u,"NCocitations",node);
}

void FSiblings(const Graph&g,node_t node,const RowVector& v)
{
 unsigned int size=g.getNbNodes();
 RowVector delta(size);delta[node]=1;
 delta=delta*g;
 ColumnVector f(size);
 for(node_t i=0;i<size;++i)f[i]=delta[i]/v[i];
 f=g*f;
 for(node_t i=0;i<size;++i)delta[i]=f[i]*v[i];//*log(1./v[i]);
 PrintNamesOfBest(g,delta,30u,"FSiblings",node);
 cout<<endl;
}

void BSiblings(const Graph&g,node_t node,const RowVector& v)
{
 unsigned int size=g.getNbNodes();
 ColumnVector fdelta(size);fdelta[node]=1/v[node];
 fdelta=g*fdelta;
 RowVector m(size);
 for(node_t i=0;i<size;++i)m[i]=fdelta[i]*v[i];
 m=m*g;
 PrintNamesOfBest(g,m,30u,"BSiblings",node);
 cout<<endl;
}

void FBSiblings(const Graph&g,node_t node,const RowVector& v)
{
 unsigned int size=g.getNbNodes();
 RowVector delta(size);delta[node]=1;
 delta=delta*g;
 ColumnVector f(size);
 for(node_t i=0;i<size;++i)f[i]=delta[i]/v[i];
 f=g*f;
 for(node_t i=0;i<size;++i)delta[i]=f[i]*v[i];
 ColumnVector fdelta(size);fdelta[node]=1/v[node];
 fdelta=g*fdelta;
 RowVector m(size);
 for(node_t i=0;i<size;++i)m[i]=fdelta[i]*v[i];
 m=m*g;
 m+=delta;
 PrintNamesOfBest(g,m,30u,"FBSiblings",node);
 cout<<endl;
}

void CosineMethod(const Graph&g,node_t node,const RowVector&)
{
 unsigned int size=g.getNbNodes();
 NodeArray spherefb;directedSphere(g,node,"FB",spherefb);
 unsigned int spheresize=count(spherefb.begin(),spherefb.end(),1),c=0;
 cerr<<spheresize<<endl;
 NodeArray nv1,nv2;vector<value_t> cosines(size);
// nodearraytf(g,node,nv1);
// for(SparseArray::iterator j=spherefb.begin(),jend=spherefb.end();
//		 j!=jend;++j,++c){
//	 nodearraytf(g,j.index(),nv2);
//	 cosines[j.index()]=cos2(nv1,nv2);
//	 if(!(c%10000))cerr<<c<<endl;
// }
// PrintNamesOfBest(g,cosines,30u);
// cout<<endl;
// cosines=vector<value_t>(size,0.);c=0;
 nodearraytfidf(g,node,nv1);
 for(SparseArray::iterator j=spherefb.begin(),jend=spherefb.end();
		 j!=jend;++j,++c){
	 nodearraytfidf(g,j.index(),nv2);
	 cosines[j.index()]=cos2(nv1,nv2);
	 if(!(c%10000))cerr<<c<<endl;
 }
 PrintNamesOfBest(g,cosines,30u,"Cosine",node);
 cout<<endl;
// cosines=vector<value_t>(size,0.);c=0;
// nodearraytfinfo(g,node,nv1,v);
// for(SparseArray::iterator j=spherefb.begin(),jend=spherefb.end();
//		 j!=jend;++j,++c){
//	 nodearraytfinfo(g,j.index(),nv2,v);
//	 cosines[j.index()]=cos2(nv1,nv2);
//	 if(!(c%10000))cerr<<c<<endl;
// }
// PrintNamesOfBest(g,cosines,30u);
}

//void Green(const Graph&g,node_t node,const RowVector&v,unsigned int nsteps=20)
//{
// cerr<<"Computing pages related to \""<<g.getLabel(node)<<"\" using the Green method"<<endl;
// node_t size=g.getNbNodes();
// RowVector greenmeasure(size),greenmeasure0(size);
// ColumnVector greenfunc(size),greenfunc0(size);Vector meangeom(size),info(size);
// greenmeasure0[node]=1;greenfunc0[node]=1;
// for(node_t j=0;j<size;j++)greenmeasure0[j]-=v[j];
// for(node_t j=0;j<size;j++)greenfunc0[j]-=v[node];
// greenmeasure=greenmeasure0;greenfunc=greenfunc0;
// for(node_t i=0;i<nsteps;++i){
//	 cout<<i<<endl;
//	 //cout<<greenmeasure.variance()<<endl;
//	 greenmeasure=greenmeasure*g;//(1./3.)*(g*greenmeasure)+(2./3.)*(h*greenmeasure);
//	 greenmeasure+=greenmeasure0;
//	 //greenfunc=g*greenfunc;
//	 //greenfunc+=greenfunc0;
//	 //SYMMETRIC CASE ONLY
//	 //for(node_t j=0;j<size;j++)greenfunc[j]=greenmeasure[j]/v[j]*v[node];
//	 //for(node_t j=0;j<size;j++)meangeom[j]=greenmeasure[j]/sqrt(v[j])*sqrt(v[node]);
//	 //for(node_t j=0;j<size;j++)meangeom[j]=greenfunc[j]*sqrt(v[j])/sqrt(v[node]);
//	 //for(node_t j=0;j<size;j++)meangeom[j]=greenfunc[j]*v[j]/v[node];
//	 for(node_t j=0;j<size;j++)info[j]=greenmeasure[j]*log(1/v[j])/log(1/v[node]);
//	 //for(node_t j=0;j<size;j++)info[j]=(greenmeasure[j]-v[j]*(greenmeasure[node]-1)/v[node])*log(1/v[j])/log(1/v[node]);
//	 //PrintNamesOfBest(g,greenfunc,30u);cout<<endl;
//	 //PrintNamesOfBest(g,meangeom,30u);cout<<endl;
//	 PrintNamesOfBest(g,info,30u,"",node);cout<<endl;
//	 //PrintNamesOfBest(g,greenmeasure,30u);cout<<endl;
//	 unsigned c=0;for(node_t j=0;j<size;j++)if(info[j]>0)++c;
//	 cout<<c<<" positive values"<<endl;
// }
//	 PrintNamesOfBest(g,info,30u,"Green",node);cout<<endl;
//}

void Green(const Graph&g,node_t node,const RowVector&v,unsigned int nsteps=20,value_t alpha=0)
{
 cerr<<"Computing pages related to \""<<g.getLabel(node)<<"\" using the Green method"<<endl;
 node_t size=g.getNbNodes();
 RowVector greenmeasure(size),deltan(size);ColumnVector ddeltan(size);
 Vector info(size);
 //Vector meangeom(size);
 deltan[node]=1;
 for(node_t i=0;i<nsteps;++i){
	 cout<<i<<endl;
	 if(alpha){
		 for(node_t j=0;j<size;j++)ddeltan[j]=deltan[j]/v[j];
		 ddeltan=g*ddeltan;
	 }
	 deltan=deltan*g;
	 if(alpha){
		 for(node_t j=0;j<size;j++)deltan[j]=(1.-alpha)*deltan[j]+alpha*ddeltan[j]*v[j];
	 }
	 deltan[node]+=1;
	 //mass(deltan)==i+2
	 for(node_t j=0;j<size;j++)greenmeasure[j]=deltan[j]-(i+2)*v[j];
	 for(node_t j=0;j<size;j++)info[j]=greenmeasure[j]*log(1/v[j])/log(1/v[node]);
	 //for(node_t j=0;j<size;j++)meangeom[j]=greenmeasure[j]/sqrt(v[j])*sqrt(v[node]);
	 PrintNamesOfBest(g,info,30u,"",node);cout<<endl;
	 //PrintNamesOfBest(g,meangeom,30u,"",node);cout<<endl;
	 //PrintNamesOfBest(g,greenmeasure,30u);cout<<endl;
	 unsigned c=0;for(node_t j=0;j<size;j++)if(deltan[j]>0)++c;
	 cout<<c<<" nonzero values"<<endl;
 }
	 PrintNamesOfBest(g,info,30u,"Green",node);cout<<endl;
}

void PersonPR(const Graph&g,node_t node,const RowVector&v,unsigned int nsteps=20)
{
 cerr<<"Computing pages related to \""<<g.getLabel(node)<<"\" using the PPR method"<<endl;
 node_t size=g.getNbNodes();
 double c=0.15;
 RowVector greenmeasure(size),greenmeasure0(size);
 Vector info(size);
 greenmeasure0[node]=1;
 greenmeasure=greenmeasure0;
 for(node_t i=0;i<nsteps;++i){
	 cout<<i<<endl;
	 greenmeasure=(1.-c)*(greenmeasure*g);//(1./3.)*(g*greenmeasure)+(2./3.)*(h*greenmeasure);
	 greenmeasure+=c*greenmeasure0;
	 //SYMMETRIC CASE ONLY
	 for(node_t j=0;j<size;j++)info[j]=1./c*greenmeasure[j]*log(1/v[j])/log(1/v[node]);
	 PrintNamesOfBest(g,info,30u,"",node);cout<<endl;
	 unsigned c=0;for(node_t j=0;j<size;j++)if(info[j]>0)++c;
	 cout<<c<<" positive values"<<endl;
 }
	 PrintNamesOfBest(g,info,30u,"PPR",node);cout<<endl;
}

void Hittingtime(const Graph&g,node_t node,const RowVector&v,unsigned int nsteps=20)
{
 cerr<<"Computing pages related to \""<<g.getLabel(node)<<"\" using the Inverse Hitting Time method"<<endl;
 node_t size=g.getNbNodes();
 ColumnVector greenfunc(size),greenfunc0(size);Vector bla(size);
 for(node_t j=0;j<size;j++)greenfunc0[j]=-1;
 greenfunc0[node]=0;
 for(node_t j=0;j<size;j++)greenfunc[j]=-100.;
 greenfunc[node]=0;
 for(node_t i=0;i<nsteps;++i){
	 cout<<i<<endl;
	 //cout<<greenmeasure.variance()<<endl;
	 greenfunc=g*greenfunc;
	 greenfunc+=greenfunc0;greenfunc[node]=0;
	 //SYMMETRIC CASE ONLY
	 //for(node_t j=0;j<size;j++)greenfunc[j]=greenmeasure[j]/v[j]*v[node];
	 //for(node_t j=0;j<size;j++)meangeom[j]=greenmeasure[j]/sqrt(v[j])*sqrt(v[node]);
	 //for(node_t j=0;j<size;j++)meangeom[j]=greenfunc[j]*sqrt(v[j])/sqrt(v[node]);
	 //for(node_t j=0;j<size;j++)meangeom[j]=greenfunc[j]*v[j]/v[node];
	 //for(node_t j=0;j<size;j++)info[j]=greenmeasure[j]*log(1/v[j])/log(1/v[node]);
	 for(node_t j=0;j<size;j++)bla[j]=v[j]*exp(greenfunc[j]);
	 //PrintNamesOfBest(g,greenfunc,30u);cout<<endl;
	 //PrintNamesOfBest(g,meangeom,30u);cout<<endl;
	 PrintNamesOfBest(g,bla,30u,"",node);cout<<endl;
	 //PrintNamesOfBest(g,greenmeasure,30u);cout<<endl;
 }
	 PrintNamesOfBest(g,bla,30u,"Hittingtime",node);cout<<endl;
}

int main(int argc, char** argv)
{
 if(argc!=3) {
	 cerr << "Usage: " << argv[0] << " word method" << endl;
	 return EXIT_FAILURE;
 }

 std::string word=argv[1];
 std::string method=argv[2];

 //  PackedGraph g(argv[1]);
 PackedGraph *pg;
 
 if(method=="GreenSym")
   pg=new PackedGraph("graph.firstscc.norm.sym.gph");
 else if(method=="Hittingtime")
   pg=new PackedGraph("graph.firstscc.norm.rev.gph");
 else
   pg=new PackedGraph("graph.firstscc.norm.gph");

 PackedGraph &g=*pg;

 // PackedGraph g("../wikipedia.firstscc.norm.gph");
 // PackedGraph sg("../wikipedia.firstscc.norm.sym.gph");
 // MutableGraph g=RandomGraph(100,0.1);

 if(!g.isOk()) {
	 cerr << "Impossible to load the graph." << endl;
	 perror("");
	 return EXIT_FAILURE;
 }

 node_t size=g.getNbNodes();
 cerr << "Number of nodes: " << size << endl;
 cerr << "Number of edges: " << g.getNbEdges() << endl;

 node_t node=g.getNodeWithLabel(argv[1]);
 if(node==(node_t)-1){cerr<<"No node with this label"<<endl;return EXIT_FAILURE;}
//  cerr<<endl<<"Edges from "<<argv[1]<<endl;
//  PrintLinksFrom(g,node);
//  cerr<<endl<<"Edges to "<<argv[1]<<endl;
//  PrintLinksTo(g,node);
//  return 0;

 cerr<<"Loading equilibrium measure"<<endl;
 RowVector v("graph.firstscc.150.msr");
 //PrintNamesOfBest(g,v,200u);return 0;

 if(method=="PageRankOfLinks")
   PageRankOfLinks(g,node,v);
 else if(method=="NeighborhoodPageRank")
   NeighborhoodPageRank(g,node,v);
 else if(method=="NCocitations")
   NCocitations(g,node);
 else if(method=="Cosine")
   CosineMethod(g,node,v);
 else if(method=="Green"||method=="GreenSym")
	 Green(g,node,v,5);
 else if(method=="BSiblings")
	 BSiblings(g,node,v);
 else if(method=="FSiblings")
	 FSiblings(g,node,v);
 else if(method=="FBSiblings")
	 FBSiblings(g,node,v);
 else if(method=="Hittingtime")
	 Hittingtime(g,node,v,10);
 else if(method=="PPR")
	 PersonPR(g,node,v,5);
 else
   throw std::logic_error("Bad method name");

// RowVector w(size);w[node]=1.;
// cout<<"1"<<endl;w=w*sg;
// cout<<"2"<<endl;w=w*sg;
// for(node_t j=0;j<size;j++)w[j]*=log(1./v[j]);
// PrintNamesOfBest(g,w,30u);

 return EXIT_SUCCESS;
}
