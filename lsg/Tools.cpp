/* $Id: Tools.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include <fstream>
#include <iterator>

#include "lsg.h"

#include "Tools.h"

using namespace std;

namespace lsg {
  bool copyFile(const string &src, const string &dst)
  {
    ifstream in(src.c_str());

    if(!in.is_open()) 
      return false;

    ofstream out(dst.c_str());

    if(!out.is_open())
      return false;

    istreambuf_iterator<char> bufReader(in); 
    ostreambuf_iterator<char> bufWriter(out); 
    copy(bufReader,istreambuf_iterator<char>(),bufWriter);

    return true;
  }
    
  void seekTillAlign(int fd,size_t size) {
    size_t pos=lseek(fd,0,SEEK_CUR);
    pos=size-(pos%size);
    if(pos!=size)
      lseek(fd,pos,SEEK_CUR);
  }

  void seekTillAlign(FILE *f,size_t size) {
    unsigned int pos=ftell(f);
    pos=size-(pos%size);
    if(pos!=size)
      fseek(f,pos,SEEK_CUR);
  }
}