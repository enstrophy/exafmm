/*
Copyright (C) 2011 by Rio Yokota, Simon Layton, Lorena Barba

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef tree_h
#define tree_h
#include "bottomup.h"

class TreeConstructor : public BottomUp {
public:
  void topdown(Bodies &bodies, Cells &cells) {
    TOPDOWN = true;
    TopDown::setDomain(bodies);
    TopDown::buildTree();
    TopDown::linkTree(bodies,cells);
    TopDown::upwardPass(cells);
  }

  void bottomup(Bodies &bodies, Cells &cells) {
    TOPDOWN = false;
    BottomUp::setDomain(bodies);
    BottomUp::buildTree(bodies,cells);
    BottomUp::linkTree(cells);
    BottomUp::upwardPass(cells);
  }
};

class SerialFMM : public TreeConstructor {
public:
  void direct(Bodies &bodies, Cells &cells) {
    setRootCell(cells);
    P2P(ROOT);
    for( B_iter B=bodies.begin(); B!=bodies.end(); ++B ) {      // Loop over bodies
      B->TRG /= B->SRC[0];
    }
  }

  void direct2(Bodies &bodies, Cells &cells) {
    setRootCell(cells);
    P2P(ROOT,ROOT,false);
    for( B_iter B=bodies.begin(); B!=bodies.end(); ++B ) {      // Loop over bodies
      B->TRG /= B->SRC[0];
    }
  }

  void approximate(Cells &cells) {
    setRootCell(cells);
    startTimer("Traverse     ");
    traverse();
    stopTimer("Traverse     ",printNow);
    startTimer("Downward pass");
    if( TOPDOWN ) {
      TopDown::downwardPass(cells);
    } else {
      BottomUp::downwardPass(cells);
    }
    stopTimer("Downward pass",printNow);
    if(printNow) printTreeData(cells);
  }

  void approximate2(Cells &cells) {
    setRootCell(cells);
    startTimer("Traverse     ");
    traverse(false);
    stopTimer("Traverse     ",printNow);
    startTimer("Downward pass");
    if( TOPDOWN ) {
      TopDown::downwardPass(cells);
    } else {
      BottomUp::downwardPass(cells);
    }
    stopTimer("Downward pass",printNow);
    if(printNow) printTreeData(cells);
  }

};

#endif
