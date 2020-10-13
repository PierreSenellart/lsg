/* $Id: TempFile.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include "TempFile.h"

#include <cstdlib>
#include <unistd.h>

using namespace std;

#ifdef __sun__
namespace {
#include <alloca.h>
#include <errno.h>
#include <sys/stat.h>

  // No mktemp in Solaris 9
  char *mkdtemp(char *tmplate)
  {
    char *t = new char[strlen(tmplate) + 1];
    char *r;

    strcpy(t, tmplate);
    for (;;) {
      r = mktemp(tmplate);

      if (*r == '\0') {
        delete [] t;
        return 0;
      }

      if (mkdir(tmplate, 0700) == 0) {
        delete [] t;
        return r;
      }

      if (errno != EEXIST) {
        delete [] t;
        return 0;
      }

      strcpy(tmplate, t);
    }
  }
}
#endif

  namespace lsg {
    TempFile::TempFile()
    {
      char rep[]="/tmp/LSGXXXXXX";
      mkdtemp(rep);
      repname=rep;

      tmp_file=repname+"/tmp";
    }

    string TempFile::name() const
    {
      return tmp_file;
    }

    TempFile::~TempFile()
    {
      unlink(tmp_file.c_str());
      rmdir(repname.c_str());
    }
  }
