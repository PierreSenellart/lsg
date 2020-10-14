#  Copyright (c) 2006 Yann Ollivier <yann.ollivier@normalesup.org>
#                     Pierre Senellart <pierre@senellart.com>
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to permit
#  persons to whom the Software is furnished to do so, subject to the
#  following conditions:
#  
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
#  NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
#  USE OR OTHER DEALINGS IN THE SOFTWARE.

CXXFLAGS=-std=c++17 -pedantic -Wall -W -Ilsg

ifdef DEBUG
CXXFLAGS+=-g
LDFLAGS+=-g
else
CXXFLAGS+=-O9 -DNDEBUG
endif

ifdef PROFILE
CXXFLAGS+=-pg
endif

CPLUS_INCLUDE_PATH=lsg
export CPLUS_INCLUDE_PATH

LIB_LSG=lsg/lsg.a

APPS=RelatedPages BuildGraphFromEdgeList ExtractFirstSCC \
     ComputeInvariantMeasure \
     Normalize Symmetrize Reverse Idftrans Statistics \
     TextVector2BinaryVector DumpSampleFiles PageRank

all: $(APPS) RunTests

%: %.o $(LIB_LSG)
	$(CXX) $(CXXFLAGS) -o $@ $^

TESTS_SRCS=$(wildcard tests/*.cpp)
RunTests: $(TESTS_SRCS:.cpp=.o) $(LIB_LSG)
	$(CXX) -o $@ $^

tests: RunTests
	./RunTests

LSG_SRCS=$(wildcard lsg/*.cpp)
$(LIB_LSG):$(LSG_SRCS:.cpp=.o)
	ar -r lsg/lsg.a $?

%.o %.P: %.cpp
	$(CXX) $(CXXFLAGS) -MD -c -o $*.o $<
	cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d

SRCS=$(wildcard *.cpp) $(wildcard */*.cpp)
-include $(SRCS:.cpp=.P)

clean:
	rm -f *.P *.o *.a */*.o */*.a */*.P

.PHONY: clean FORCE
