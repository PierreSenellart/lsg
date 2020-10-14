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
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <set>
#include <map>

#include "ConnectedComponents.h"
#include "SparseArray.h"
#include "MarkovChains.h"
#include "Vector.h"
#include "NodeArray.h"

using namespace std;
using namespace lsg;

const double precision=.001;
const string HIGHLIGHT="blue";
const string STRONG_HIGHLIGHT="darkorange";

void dumpGraph(
    const string &filename,
    const Graph &g,
    const Vector *weights=0,
    const map<node_t,string> *highlighted=0,
    const set<node_t> *hidden=0,
    const map<pair<node_t,node_t>,string> *highlighted_edges=0)
{
  ofstream f(filename.c_str());
  f << "<?xml version='1.0' encoding='utf-8'?>\n";
  f << "<graph>\n";

  node_t size=g.getNbNodes();

  for(node_t i=0;i<size;++i) {
    f << "  <node id='" << i << "'";

    if(weights) {
      f << " weight='";
      if(!isnan(static_cast<long double>((*weights)[i])))
        f << (*weights)[i];
      f << "'";
    }

    if(highlighted && highlighted->find(i)!=highlighted->end()) {
      f << " highlighted='" << highlighted->find(i)->second << "'";
    }
    
    if(hidden && hidden->find(i)!=hidden->end()) {
      f << " hidden='1'";
    }
    
    f << ">\n";

    for(SparseArray::const_iterator it=g.row(i).begin(),
                                itend=g.row(i).end();
        it!=itend;
        ++it) {
      f << "    <edge to='" << it.index()
        << "' weight='" << it.value() << "'";

      if((!highlighted_edges && highlighted &&
           highlighted->find(i)!=highlighted->end() &&
           highlighted->find(it.index())!=highlighted->end()) ||
         (highlighted_edges &&
          highlighted_edges->find(make_pair(i,it.index()))!=
            highlighted_edges->end()))
        f << " highlighted='" <<
          (highlighted_edges?
           highlighted_edges->find(make_pair(i,it.index()))->second:
           (highlighted->find(it.index())->second==
            highlighted->find(i)->second?
            highlighted->find(i)->second:
            HIGHLIGHT))
            << "'";

      f << " />\n";
    }

    f << "  </node>\n";
  }

  f << "</graph>\n";
}

void Green(const Graph&g,node_t node,const RowVector&v,const string &prefix)
{
  node_t size=g.getNbNodes();
  RowVector greenmeasure(size),deltan(size);
  Vector info(size);
  deltan[node]=1;
  RowVector oldgreenmeasure(greenmeasure);
  node_t i=0;
  do {
    stringstream s;
    s << i;
    for(node_t j=0;j<size;j++)greenmeasure[j]=deltan[j]-(i+1)*v[j];
    oldgreenmeasure=greenmeasure;
    dumpGraph(prefix+"-"+s.str()+".xml",g,&greenmeasure);

    deltan=deltan*g;
    deltan[node]+=1;
    for(node_t j=0;j<size;j++)greenmeasure[j]=deltan[j]-(i+2)*v[j];
    for(node_t j=0;j<size;j++)info[j]=greenmeasure[j]*log(1/v[j])/log(1/v[node]);
    node_t c=0;for(node_t j=0;j<size;j++)if(deltan[j]>0)++c;

    ++i;
  } while(abs(oldgreenmeasure-greenmeasure).max()>precision/2);
}

void SomeNeighborhood(const Graph&g, node_t node, NodeArray& a, const string &prefix)
{
  map<node_t,string> highlighted;
  highlighted[node]=STRONG_HIGHLIGHT;
  dumpGraph(prefix+"0.xml",g,0,&highlighted);

  NodeArray n;
  directedSphere(g,node,"",a);
  directedSphere(g,node,"F",n);
  addNodeArray(a,n,a);

  for(node_t i=0;i<g.getNbNodes();++i)
    if(n.find(i)!=n.end() && i!=node)
      highlighted[i]=HIGHLIGHT;
  dumpGraph(prefix+"1.xml",g,0,&highlighted);

  directedSphere(g,node,"FB",n);
  addNodeArray(a,n,a);
  
  for(node_t i=0;i<g.getNbNodes();++i)
    if(n.find(i)!=n.end() && i!=node)
      highlighted[i]=HIGHLIGHT;
  dumpGraph(prefix+"2.xml",g,0,&highlighted);
  
  directedSphere(g,node,"B",n);
  addNodeArray(a,n,a);

  for(node_t i=0;i<g.getNbNodes();++i)
    if(n.find(i)!=n.end() && i!=node)
      highlighted[i]=HIGHLIGHT;
  dumpGraph(prefix+"3.xml",g,0,&highlighted);

  directedSphere(g,node,"BF",n);
  addNodeArray(a,n,a);
  
  for(node_t i=0;i<g.getNbNodes();++i)
    if(n.find(i)!=n.end() && i!=node)
      highlighted[i]=HIGHLIGHT;
  dumpGraph(prefix+"4.xml",g,0,&highlighted);
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

double cosine(const Graph&g, node_t node1, node_t node2)
{
  NodeArray nv1,nv2;
  nodearraytfidf(g,node1,nv1);
  nodearraytfidf(g,node2,nv2);
  return cos2(nv1,nv2);
}

int main(int argc, char** argv)
{
  if(argc!=4) {
    cerr << "Usage: " << argv[0]
      << " edges directory node" << endl;
    return EXIT_FAILURE;
  }

  string edges=argv[1];
  string dir=argv[2];
  dir+="/";

  node_t node;
  stringstream ss;
  ss << argv[3];
  ss >> node;

  ifstream f(edges.c_str());
  MutableGraph g(f);
  f.close();

  if(!g.isOk()) {
    cerr << "Impossible to load the graph." << endl;
    return EXIT_FAILURE;
  }

  {
    map<node_t,string> highlighted;
    highlighted[node]=STRONG_HIGHLIGHT;
    dumpGraph(dir+"graphhigh.xml",g,0,&highlighted);
  }

  stochastifyRows(g);
  dumpGraph(dir+"stochastified.xml",g);

  node_t size=g.getNbNodes();
  RowVector pr(size);

  cerr << "Number of nodes: " << size << endl;
  cerr << "Number of edges: " << g.getNbEdges() << endl;

  cerr << "PageRank..." << endl;

  {
    for(node_t i=0;i<size;++i)
      pr[i]=1./size;

    RowVector oldpr(pr);
    node_t i=0;
    do {
      stringstream s;
      s << i;
      dumpGraph(dir+"pr-"+s.str()+".xml",g,&pr);
      oldpr=pr;
      InvariantMeasure(g,pr,1,false);
      ++i;
    } while(abs(pr-oldpr).max()>precision/2);
  }

  {
    cerr << "Green..." << endl;
    Green(g,node,pr,dir+"green");
  }

  {
    cerr << "SymGreen..." << endl;
    MutableGraph gsym(g);
    Symmetrize(gsym,pr);
    dumpGraph(dir+"symgraph.xml",gsym);
    Green(gsym,node,pr,dir+"symgreen");
  }

  {
    cerr << "PageRankOfLinks..." << endl;
    map<node_t,string> highlighted;
    highlighted[node]=STRONG_HIGHLIGHT;

    dumpGraph(dir+"prfinal.xml",g,&pr);
    dumpGraph(dir+"prfinalhigh.xml",g,&pr,&highlighted);

    map<pair<node_t, node_t>,string> highlighted_edges;

    for(SparseArray::iterator it=g.row(node).begin(),
        itend=g.row(node).end();
        it!=itend;
        ++it) {
      highlighted[it.index()]=HIGHLIGHT;
      highlighted_edges[make_pair(node,it.index())]=HIGHLIGHT;
    }

    dumpGraph(dir+"prlinks.xml",g,&pr,&highlighted,0,&highlighted_edges);
  }

  {
    cerr << "LocalPageRank..." << endl;
    NodeArray n;
    SomeNeighborhood(g,node,n,dir+"lprsubg");
    
    set<node_t> hidden;
    for(node_t i=0;i<size;++i)
      if(n.find(i)==n.end())
        hidden.insert(i);

    dumpGraph(dir+"lprsubghid.xml",g,0,0,&hidden);

    vector<bool> whichnodes(size);
    map<node_t,node_t> translation;
    for(node_t i=0,j=0;i<size;++i)
      if(n.find(i)!=n.end()) {
        whichnodes[i]=true;
        translation[i]=j++;
      }

    MutableGraph h(g,whichnodes);
    vector<node_t> comp(h.getNbNodes());
    stronglyConnectedComponents(h,comp);

    vector<bool> whichnodes2(size);
    map<node_t,node_t> translation2;
    map<node_t,string> highlighted;
    for(node_t i=0,j=0;i<h.getNbNodes();++i)
      if(comp[i]==comp[translation[node]]) {
        whichnodes2[i]=true;
        translation2[i]=j++;
      }

    for(node_t i=0;i<size;++i)
      if(whichnodes[i] && whichnodes2[translation[i]])
        highlighted[i]=HIGHLIGHT;

    dumpGraph(dir+"lprsubgscc.xml",g,0,&highlighted,&hidden);
    
    for(node_t i=0;i<size;++i)
      if(whichnodes[i] && !whichnodes2[translation[i]])
        hidden.insert(i);

    dumpGraph(dir+"lprsubgscchid.xml",g,0,0,&hidden);
    MutableGraph h2(h,whichnodes2);
    stochastifyRows(h2);
  
    RowVector pr2(h2.getNbNodes());
    for(node_t i=0;i<h2.getNbNodes();++i)
      pr2[i]=1./h2.getNbNodes();

    RowVector oldpr2(pr2);
    node_t i=0;
    do {
      stringstream s;
      s << i;

      RowVector newpr(size);
      for(node_t j=0;j<size;++j)
        if(whichnodes[j] && whichnodes2[translation[j]])
          newpr[j]=pr2[translation2[translation[j]]];
        else
          newpr[j]=0./0.;

      dumpGraph(dir+"lpr-"+s.str()+".xml",g,&newpr,0,&hidden);
      oldpr2=pr2;
      InvariantMeasure(h2,pr2,1,false);
      ++i;
    } while(abs(pr2-oldpr2).max()>precision/2);
  }
  
  {
    cerr << "Cosine..." << endl;
    map<node_t,string> highlighted;
    highlighted[node]=STRONG_HIGHLIGHT;

    map<pair<node_t, node_t>,string> highlighted_edges;

    for(SparseArray::iterator it=g.row(node).begin(),
        itend=g.row(node).end();
        it!=itend;
        ++it) {
      highlighted[it.index()]=HIGHLIGHT;
      highlighted_edges[make_pair(node,it.index())]=HIGHLIGHT;
    }
    
    dumpGraph(dir+"forward.xml",g,0,&highlighted,0,&highlighted_edges);
    
    highlighted.erase(node);
    highlighted_edges.clear();

    set<node_t> dimensions;
    set<node_t> documents;

    std::map<node_t,string> highlighted2;
    for(map<node_t,string>::iterator it=highlighted.begin(),
                                   itend=highlighted.end();
        it!=itend;
        ++it)
      for(SparseArray::iterator jt=g.column(it->first).begin(),
          jtend=g.column(it->first).end();
          jt!=jtend;
          ++jt) {
        highlighted2[jt.index()]=STRONG_HIGHLIGHT;
        highlighted_edges[make_pair(jt.index(),it->first)]=HIGHLIGHT;
        documents.insert(jt.index());
        for(SparseArray::iterator kt=g.row(jt.index()).begin(),
                               ktend=g.row(jt.index()).end();
            kt!=ktend;
            ++kt) {
          dimensions.insert(kt.index());
        }
      }
    highlighted.insert(highlighted2.begin(),highlighted2.end());

    dumpGraph(dir+"cosine.xml",g,0,&highlighted,0,&highlighted_edges);

    ofstream o((dir+"cosine.tex").c_str());
    o << "\\begin{tabular}{cc|";
    for(node_t i=0;i<dimensions.size();++i)
      o << "c|";
    o << "|c|}\n";
    o << "&\\multicolumn{1}{c}{}&"
      << "\\multicolumn{"
      << dimensions.size() << "}{c}{\\structure{Dimensions}}\\\\\n";
    o << "\\cline{3-" << dimensions.size()+3 << "}\n";

    o << "&";
    for(set<node_t>::const_iterator it=dimensions.begin(),
                                  itend=dimensions.end();
        it!=itend;
        ++it)
      o << "&" << *it+1;
    o << "&Cosine with " << node+1 << "\\\\\n\\cline{2-"
      << dimensions.size()+3 << "}\n";

    o << "\\multirow{" << documents.size() << "}{*}"
      << "{\\begin{sideways}\\structure{Documents}\\end{sideways}}\n";

    for(set<node_t>::const_iterator it=documents.begin(),
                                  itend=documents.end();
        it!=itend;
        ++it) {
      o << "&\\multicolumn{1}{|c|}{"
        << (*it==node?"\\alert{":"") << *it+1
        << (*it==node?"}":"") << "}";
      for(set<node_t>::const_iterator jt=dimensions.begin(),
                                    jtend=dimensions.end();
          jt!=jtend;
          ++jt) {
        o << "&" ;

        if(g(*it,*jt))
          o << (*it==node?"\\alert{":"") << "\\checkmark"
            << (*it==node?"}":"");
      }

      double c=cosine(g,node,*it);
      char cos[5];
      sprintf(cos,"%.2f",c);

      o << "&" << (*it==node?"\\alert{":"") << cos << (*it==node?"}":"");
      o<<"\\\\\n\\cline{2-" << dimensions.size()+3 << "}\n";
    }

    o << "\\end{tabular}";
  }

  {
    cerr << "Cocitations..." << endl;

    map<node_t,string> highlighted;
    map<pair<node_t, node_t>,string> highlighted_edges;

    highlighted[node]=STRONG_HIGHLIGHT;
    for(SparseArray::iterator it=g.column(node).begin(),
        itend=g.column(node).end();
        it!=itend;
        ++it) {
      highlighted[it.index()]=HIGHLIGHT;
      highlighted_edges[make_pair(it.index(),node)]=HIGHLIGHT;
    }

    dumpGraph(dir+"citations.xml",g,0,&highlighted,0,&highlighted_edges);
    
    highlighted.erase(node);
    highlighted_edges.clear();

    map<node_t,string> highlighted2;
    for(map<node_t,string>::iterator it=highlighted.begin(),
                                   itend=highlighted.end();
        it!=itend;
        ++it) {
      it->second=STRONG_HIGHLIGHT;
      for(SparseArray::iterator jt=g.row(it->first).begin(),
          jtend=g.row(it->first).end();
          jt!=jtend;
          ++jt) {
        highlighted2[jt.index()]=HIGHLIGHT;
        highlighted_edges[make_pair(it->first,jt.index())]=HIGHLIGHT;
      }
    }

    highlighted.insert(highlighted2.begin(),highlighted2.end());

    dumpGraph(dir+"cocitations.xml",g,0,&highlighted,0,&highlighted_edges);
  }

  return EXIT_SUCCESS;
}
