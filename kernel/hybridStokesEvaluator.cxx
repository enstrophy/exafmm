/*
 * Copyright (C) 2011 by Rio Yokota, Simon Layton, Lorena Barba
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef STOKES
void Evaluator<Stokes>::setSourceBody()                       // Set source buffer for bodies
{
    startTimer("Set sourceB  ");                                  // Start timer
    for (MC_iter M = sourceSize.begin(); M != sourceSize.end(); ++M)  // Loop over source map
    {
        C_iter Cj = M->first;                                       //  Set source cell
        sourceBegin[Cj] = sourceHost.size() / 6;                    //  Key : iterator, Value : offset of source leafs
        for (B_iter B = Cj->LEAF; B != Cj->LEAF + Cj->NDLEAF; ++B)  //  Loop over leafs in source cell
        {
            sourceHost.push_back(B->X[0]);                            //   Copy x position to GPU buffer
            sourceHost.push_back(B->X[1]);                            //   Copy y position to GPU buffer
            sourceHost.push_back(B->X[2]);                            //   Copy z position to GPU buffer
            sourceHost.push_back(B->FORCE[0]);                             //   Copy source value to GPU buffer
            sourceHost.push_back(B->FORCE[1]);                             //   Copy source value to GPU buffer
            sourceHost.push_back(B->FORCE[2]);                             //   Copy source value to GPU buffer
        }                                                           //  End loop over leafs
    }                                                             // End loop over source map
    stopTimer("Set sourceB  ");                                   // Stop timer
}
#endif

void Evaluator<Stokes>::setTargetBody(Lists lists, Maps flags)  // Set target buffer for bodies
{
    startTimer("Set targetB  ");                                  // Start timer
    int key = 0;                                                  // Initialize key to range of coefs in source cells
    for (C_iter Ci = CiB; Ci != CiE; ++Ci)                        // Loop over target cells
    {
        if (!lists[Ci-Ci0].empty())                                 //  If the interation list is not empty
        {
            int blocks = (Ci->NDLEAF - 1) / THREADS + 1;              //   Number of thread blocks needed for this target cell
            for (int i = 0; i != blocks; ++i)                         //   Loop over thread blocks
            {
                keysHost.push_back(key);                                //    Save key to range of leafs in source cells
            }                                                         //   End loop over thread blocks
            key += 3 * lists[Ci-Ci0].size() + 1;                      //   Increment key counter
            rangeHost.push_back(lists[Ci-Ci0].size());                //   Save size of interaction list
            for (LC_iter L = lists[Ci-Ci0].begin(); L != lists[Ci-Ci0].end(); ++L)  //  Loop over interaction list
            {
                C_iter Cj = *L;                                         //   Set source cell
                rangeHost.push_back(sourceBegin[Cj]);                   //    Set begin index of coefs in source cell
                rangeHost.push_back(sourceSize[Cj]);                    //    Set number of coefs in source cell
                rangeHost.push_back(flags[Ci-Ci0][Cj]);                 //    Set periodic image flag of source cell
            }                                                         //   End loop over interaction list
            targetBegin[Ci] = targetHost.size() / 4;                  //   Key : iterator, Value : offset of target leafs
            for (B_iter B = Ci->LEAF; B != Ci->LEAF + Ci->NDLEAF; ++B)     //   Loop over leafs in target cell
            {
                targetHost.push_back(B->X[0]);                          //    Copy x position to GPU buffer
                targetHost.push_back(B->X[1]);                          //    Copy y position to GPU buffer
                targetHost.push_back(B->X[2]);                          //    Copy z position to GPU buffer
                targetHost.push_back(B->SRC);                           //    Copy target value to GPU buffer
            }                                                         //   End loop over leafs
            int numPad = blocks * THREADS - Ci->NDLEAF;               //   Number of elements to pad in target GPU buffer
            for (int i = 0; i != numPad; ++i)                         //   Loop over elements to pad
            {
                targetHost.push_back(0);                                //    Pad x position in GPU buffer
                targetHost.push_back(0);                                //    Pad y position in GPU buffer
                targetHost.push_back(0);                                //    Pad z position in GPU buffer
                targetHost.push_back(0);                                //    Pad target value to GPU buffer
            }                                                         //   End loop over elements to pad
        }                                                           //  End if for empty interation list
    }                                                             // End loop over target cells
    stopTimer("Set targetB  ");                                   // Stop timer
}

void Evaluator<Stokes>::getTargetBody(Lists &lists)           // Get body values from target buffer
{
    startTimer("Get targetB  ");                                  // Start timer
    for (C_iter Ci = CiB; Ci != CiE; ++Ci)                        // Loop over target cells
    {
        if (!lists[Ci-Ci0].empty())                                 //  If the interation list is not empty
        {
            int begin = targetBegin[Ci];                              //   Offset of target leafs
            for (B_iter B = Ci->LEAF; B != Ci->LEAF + Ci->NDLEAF; ++B)   //    Loop over target bodies
            {
                B->TRG[0] += targetHost[4*(begin+B-Ci->LEAF)+0];      //     Copy 1st target value from GPU buffer
                B->TRG[1] += targetHost[4*(begin+B-Ci->LEAF)+1];      //     Copy 2nd target value from GPU buffer
                B->TRG[2] += targetHost[4*(begin+B-Ci->LEAF)+2];      //     Copy 3rd target value from GPU buffer
                B->TRG[3] += targetHost[4*(begin+B-Ci->LEAF)+3];      //     Copy 4th target value from GPU buffer
            }                                                       //    End loop over target bodies
            lists[Ci-Ci0].clear();                                    //   Clear interaction list
        }                                                           //  End if for empty interation list
    }                                                             // End loop over target cells
    stopTimer("Get targetB  ");                                   // Stop timer
}

void Evaluator<Stokes>::clearBuffers()                        // Clear GPU buffers
{
    startTimer("Clear buffer ");                                  // Start timer
    constHost.clear();                                            // Clear const vector
    keysHost.clear();                                             // Clear keys vector
    rangeHost.clear();                                            // Clear range vector
    sourceHost.clear();                                           // Clear source vector
    targetHost.clear();                                           // Clear target vector
    sourceBegin.clear();                                          // Clear map for offset of source cells
    sourceSize.clear();                                           // Clear map for size of source cells
    targetBegin.clear();                                          // Clear map for offset of target cells
    stopTimer("Clear buffer ");                                   // Stop timer
}
#ifdef STOKES
void Evaluator<Stokes>::evalP2P(Bodies &ibodies, Bodies &jbodies, bool onCPU)  // Evaluate all P2P kernels
{
    int numIcall = int(ibodies.size() - 1) / MAXBODY + 1;         // Number of icall loops
    int numJcall = int(jbodies.size() - 1) / MAXBODY + 1;         // Number of jcall loops
    int ioffset = 0;                                              // Initialzie offset for icall loops
    Cells cells(2);                                               // Two cells to put target and source bodies
    for (int icall = 0; icall != numIcall; ++icall)               // Loop over icall
    {
        B_iter Bi0 = ibodies.begin() + ioffset;                     //  Set target bodies begin iterator
        B_iter BiN = ibodies.begin() + std::min(ioffset + MAXBODY, int(ibodies.size()));// Set target bodies end iterator
        cells[0].LEAF = Bi0;                                        //  Iterator of first target leaf
        cells[0].NDLEAF = BiN - Bi0;                                //  Number of target leafs
        int joffset = 0;                                            //  Initialize offset for jcall loops
        for (int jcall = 0; jcall != numJcall; ++jcall)             //  Loop over jcall
        {
            B_iter Bj0 = jbodies.begin() + joffset;                   //  Set source bodies begin iterator
            B_iter BjN = jbodies.begin() + std::min(joffset + MAXBODY, int(jbodies.size()));// Set source bodies end iterator
            cells[1].LEAF = Bj0;                                      //  Iterator of first source leaf
            cells[1].NDLEAF = BjN - Bj0;                              //  Number of source leafs
            C_iter Ci = cells.begin(), Cj = cells.begin() + 1;        //  Iterator of target and source cells
            if (onCPU)                                                //  If calculation is to be done on CPU
            {
                Xperiodic = 0;                                          //   Set periodic coordinate offset
                P2P(Ci, Cj);                                            //   Perform P2P kernel on CPU
            }
            else                                                    //  If calculation is to be done on GPU
            {
                constHost.push_back(2*R0);                              //   Copy domain size to GPU buffer
                for (B_iter B = Bj0; B != BjN; ++B)                     //   Loop over source bodies
                {
                    sourceHost.push_back(B->X[0]);                        //   Copy x position to GPU buffer
                    sourceHost.push_back(B->X[1]);                        //   Copy y position to GPU buffer
                    sourceHost.push_back(B->X[2]);                        //   Copy z position to GPU buffer
                    sourceHost.push_back(B->FORCE[0]);                         //   Copy source value to GPU buffer
                    sourceHost.push_back(B->FORCE[1]);                         //   Copy source value to GPU buffer
                    sourceHost.push_back(B->FORCE[2]);                         //   Copy source value to GPU buffer
                }                                                       //   End loop over source bodies
                int key = 0;                                            //   Initialize key to range of leafs in source cells
                int blocks = (BiN - Bi0 - 1) / THREADS + 1;             //   Number of thread blocks needed for this target cell
                for (int i = 0; i != blocks; ++i)                       //   Loop over thread blocks
                {
                    keysHost.push_back(key);                              //    Save key to range of leafs in source cells
                }                                                       //   End loop over thread blocks
                rangeHost.push_back(1);                                 //   Save size of interaction list
                rangeHost.push_back(0);                                 //   Set begin index of leafs
                rangeHost.push_back(BjN - Bj0);                         //   Set number of leafs
                rangeHost.push_back(Icenter);                           //   Set periodic image flag
                for (B_iter B = Bi0; B != BiN; ++B)                     //   Loop over target bodies
                {
                    targetHost.push_back(B->X[0]);                        //    Copy x position to GPU buffer
                    targetHost.push_back(B->X[1]);                        //    Copy y position to GPU buffer
                    targetHost.push_back(B->X[2]);                        //    Copy z position to GPU buffer
                    targetHost.push_back(B->SRC);                         //    Copy target value to GPU buffer
                }                                                       //   End loop over target bodies
                int numPad = blocks * THREADS - (BiN - Bi0);            //   Number of elements to pad in target GPU buffer
                for (int i = 0; i != numPad; ++i)                       //   Loop over elements to pad
                {
                    targetHost.push_back(0);                              //    Pad x position in GPU buffer
                    targetHost.push_back(0);                              //    Pad y position in GPU buffer
                    targetHost.push_back(0);                              //    Pad z position in GPU buffer
                    targetHost.push_back(0);                              //    Pad target value to GPU buffer
                }                                                       //   End loop over elements to pad
                allocate();                                             //   Allocate GPU memory
                hostToDevice();                                         //   Copy from host to device
                P2P();                                                  //   Perform P2P kernel
                deviceToHost();                                         //   Copy from device to host
                for (B_iter B = Bi0; B != BiN; ++B)                     //   Loop over target bodies
                {
                    B->TRG[0] += targetHost[4*(B-Bi0)+0];                 //    Copy 1st target value from GPU buffer
                    B->TRG[1] += targetHost[4*(B-Bi0)+1];                 //    Copy 2nd target value from GPU buffer
                    B->TRG[2] += targetHost[4*(B-Bi0)+2];                 //    Copy 3rd target value from GPU buffer
                    B->TRG[3] += targetHost[4*(B-Bi0)+3];                 //    Copy 4th target value from GPU buffer
                }                                                       //   End loop over target bodies
                keysHost.clear();                                       //   Clear keys vector
                rangeHost.clear();                                      //   Clear range vector
                constHost.clear();                                      //   Clear const vector
                targetHost.clear();                                     //   Clear target vector
                sourceHost.clear();                                     //   Clear source vector
            }                                                         //  Endif for CPU/GPU switch
            joffset += MAXBODY;                                       //  Increment jcall offset
        }                                                           // End loop over jcall
        ioffset += MAXBODY;                                         // Increment icall offset
    }                                                             // End loop over icall
}
#endif

void Evaluator<Stokes>::evalP2M(Cells &cells)                 // Evaluate all P2M kernels
{
    startTimer("evalP2M      ");                                  // Start timer
    for (C_iter C = cells.begin(); C != cells.end(); ++C)         // Loop over cells
    {
        C->M = 0;                                                   //  Initialize multipole coefficients
        C->L = 0;                                                   //  Initialize local coefficients
        if (C->NCHILD == 0)                                         //  If cell is a twig
        {
            P2M(C);                                                   //   Perform P2M kernel
        }                                                           //  Endif for twig
    }                                                             // End loop over cells
    stopTimer("evalP2M      ");                                   // Stop timer
}


void Evaluator<Stokes>::evalM2M(Cells &cells, Cells &jcells)  // Evaluate all M2M kernels
{
    startTimer("evalM2M      ");                                  // Start timer
    Cj0 = jcells.begin();                                         // Set begin iterator
    for (C_iter Ci = cells.begin(); Ci != cells.end(); ++Ci)      // Loop over target cells bottomup
    {
        for (C_iter Cj = Cj0 + Ci->CHILD; Cj != Cj0 + Ci->CHILD + Ci->NCHILD; ++Cj)  // Loop over child cells
        {
            M2M(Ci, Cj);                                              //   Perform M2M kernel
        }                                                           //  End loop over child cells
    }                                                             // End loop target over cells
    stopTimer("evalM2M      ");                                   // Stop timer
}


void Evaluator<Stokes>::evalM2L(C_iter Ci, C_iter Cj)         // Evaluate single M2L kernel
{
    startTimer("evalM2L      ");
    M2L(Ci, Cj);                                                  // Perform M2L kernel
    stopTimer("evalM2L      ");
    NM2L++;                                                       // Count M2L kernel execution
}


void Evaluator<Stokes>::evalM2L(Cells &cells)                 // Evaluate queued M2L kernels
{
    startTimer("evalM2L      ");                                  // Start timer
    Ci0 = cells.begin();                                          // Set begin iterator
    for (C_iter Ci = cells.begin(); Ci != cells.end(); ++Ci)      // Loop over cells
    {
        while (!listM2L[Ci-Ci0].empty())                            //  While M2L interaction list is not empty
        {
            C_iter Cj = listM2L[Ci-Ci0].back();                       //   Set source cell iterator
            Iperiodic = flagM2L[Ci-Ci0][Cj];                          //   Set periodic image flag
            int I = 0;                                                //   Initialize index of periodic image
            for (int ix = -1; ix <= 1; ++ix)                          //   Loop over x periodic direction
            {
                for (int iy = -1; iy <= 1; ++iy)                        //    Loop over y periodic direction
                {
                    for (int iz = -1; iz <= 1; ++iz, ++I)                 //     Loop over z periodic direction
                    {
                        if (Iperiodic & (1 << I))                           //      If periodic flag is on
                        {
                            Xperiodic[0] = ix * 2 * R0;                       //       Coordinate offset for x periodic direction
                            Xperiodic[1] = iy * 2 * R0;                       //       Coordinate offset for y periodic direction
                            Xperiodic[2] = iz * 2 * R0;                       //       Coordinate offset for z periodic direction
                            M2L(Ci, Cj);                                      //       Perform M2L kernel
                            NM2L++;       
                        }                                                   //      Endif for periodic flag
                    }                                                     //     End loop over x periodic direction
                }                                                       //    End loop over y periodic direction
            }                                                         //   End loop over z periodic direction
            listM2L[Ci-Ci0].pop_back();                               //   Pop last element from M2L interaction list
        }                                                           //  End while for M2L interaction list
    }                                                             // End loop over cells topdown
    listM2L.clear();                                              // Clear interaction lists
    flagM2L.clear();                                              // Clear periodic image flags
    stopTimer("evalM2L      ");                                   // Stop timer
}


void Evaluator<Stokes>::evalM2P(C_iter Ci, C_iter Cj)         // Evaluate single M2P kernel
{
    startTimer("evalM2P      ");                                  // Start timer
    M2P(Ci, Cj);                                                  // Perform M2P kernel
    stopTimer("evalM2P      ");                                   // Stop timer
    NM2P++;                                                       // Count M2P kernel execution
}


void Evaluator<Stokes>::evalM2P(Cells &cells)                 // Evaluate queued M2P kernels
{
    startTimer("evalM2P      ");                                  // Start timer
    Ci0 = cells.begin();                                          // Set begin iterator
    for (C_iter Ci = cells.begin(); Ci != cells.end(); ++Ci)      // Loop over cells
    {
        while (!listM2P[Ci-Ci0].empty())                            //  While M2P interaction list is not empty
        {
            C_iter Cj = listM2P[Ci-Ci0].back();                       //   Set source cell iterator
            Iperiodic = flagM2P[Ci-Ci0][Cj];                          //   Set periodic image flag
            int I = 0;                                                //   Initialize index of periodic image
            for (int ix = -1; ix <= 1; ++ix)                          //   Loop over x periodic direction
            {
                for (int iy = -1; iy <= 1; ++iy)                        //    Loop over y periodic direction
                {
                    for (int iz = -1; iz <= 1; ++iz, ++I)                 //     Loop over z periodic direction
                    {
                        if (Iperiodic & (1 << I))                           //      If periodic flag is on
                        {
                            Xperiodic[0] = ix * 2 * R0;                       //       Coordinate offset for x periodic direction
                            Xperiodic[1] = iy * 2 * R0;                       //       Coordinate offset for y periodic direction
                            Xperiodic[2] = iz * 2 * R0;                       //       Coordinate offset for z periodic direction
                            M2P(Ci, Cj);                                      //       Perform M2P kernel
                            NM2P++;
                        }                                                   //      Endif for periodic flag
                    }                                                     //     End loop over x periodic direction
                }                                                       //    End loop over y periodic direction
            }                                                         //   End loop over z periodic direction
            listM2P[Ci-Ci0].pop_back();                               //   Pop last element from M2P interaction list
        }                                                           //  End while for M2P interaction list
    }                                                             // End loop over cells topdown
    listM2P.clear();                                              // Clear interaction lists
    flagM2P.clear();                                              // Clear periodic image flags
    stopTimer("evalM2P      ");                                   // Stop timer
}


void Evaluator<Stokes>::evalP2P(C_iter Ci, C_iter Cj)         // Queue single P2P kernel
{
    listP2P[Ci-Ci0].push_back(Cj);                                // Push source cell into P2P interaction list
    flagP2P[Ci-Ci0][Cj] |= Iperiodic;                             // Flip bit of periodic image flag
    NP2P++;                                                       // Count P2P kernel execution
}


void Evaluator<Stokes>::evalP2P(Cells &cells)                 // Evaluate queued P2P kernels
{
    Ci0 = cells.begin();                                          // Set begin iterator
    const int numCell = MAXCELL / NCRIT / 4;                      // Number of cells per icall
    int numIcall = int(cells.size() - 1) / numCell + 1;           // Number of icall loops
    int ioffset = 0;                                              // Initialzie offset for icall loops
    for (int icall = 0; icall != numIcall; ++icall)               // Loop over icall
    {
        CiB = cells.begin() + ioffset;                              //  Set begin iterator for target per call
        CiE = cells.begin() + std::min(ioffset + numCell, int(cells.size()));// Set end iterator for target per call
        constHost.push_back(2*R0);                                  //  Copy domain size to GPU buffer
        startTimer("Get list     ");                                //  Start timer
        for (C_iter Ci = CiB; Ci != CiE; ++Ci)                      //  Loop over target cells
        {
            for (LC_iter L = listP2P[Ci-Ci0].begin(); L != listP2P[Ci-Ci0].end(); ++L)  //  Loop over interaction list
            {
                C_iter Cj = *L;                                         //    Set source cell
                sourceSize[Cj] = Cj->NDLEAF;                            //    Key : iterator, Value : number of leafs
            }                                                         //   End loop over interaction list
        }                                                           //  End loop over target cells
        stopTimer("Get list     ");                                 //  Stop timer
        setSourceBody();                                            //  Set source buffer for bodies
        setTargetBody(listP2P, flagP2P);                            //  Set target buffer for bodies
        allocate();                                                 //  Allocate GPU memory
        hostToDevice();                                             //  Copy from host to device
        P2P();                                                      //  Perform P2P kernel
        deviceToHost();                                             //  Copy from device to host
        getTargetBody(listP2P);                                     //  Get body values from target buffer
        clearBuffers();                                             //  Clear GPU buffers
        ioffset += numCell;                                         //  Increment ioffset
    }                                                             // End loop over icall
    listP2P.clear();                                              // Clear interaction lists
    flagP2P.clear();                                              // Clear periodic image flags
}


void Evaluator<Stokes>::evalL2L(Cells &cells)                 // Evaluate all L2L kernels
{
    startTimer("evalL2L      ");                                  // Start timer
    Ci0 = cells.begin();                                          // Set begin iterator
    for (C_iter Ci = cells.end() - 2; Ci != cells.begin() - 1; --Ci)     // Loop over cells topdown (except root cell)
    {
        C_iter Cj = Ci0 + Ci->PARENT;                               //  Set source cell iterator
        L2L(Ci, Cj);                                                //  Perform L2L kernel
    }                                                             // End loop over cells topdown
    stopTimer("evalL2L      ");                                   // Stop timer
}


void Evaluator<Stokes>::evalL2P(Cells &cells)                 // Evaluate all L2P kernels
{
    startTimer("evalL2P      ");                                  // Start timer
    for (C_iter C = cells.begin(); C != cells.end(); ++C)         // Loop over cells
    {
        if (C->NCHILD == 0)                                         //  If cell is a twig
        {
            L2P(C);                                                   //   Perform L2P kernel
        }                                                           //  Endif for twig
    }                                                             // End loop over cells topdown
    stopTimer("evalL2P      ");                                   // Stop timer
}


void Evaluator<Stokes>::timeKernels()                         // Time all kernels for auto-tuning
{
    Bodies ibodies(1000), jbodies(1000);                          // Artificial bodies
    for (B_iter Bi = ibodies.begin(), Bj = jbodies.begin(); Bi != ibodies.end(); ++Bi, ++Bj)  // Loop over artificial bodies
    {
        Bi->X = 0;                                                  //  Set coordinates of target body
        Bj->X = 1;                                                  //  Set coordinates of source body
    }                                                             // End loop over artificial bodies
    Cells cells;                                                  // Artificial cells
    cells.resize(2);                                              // Two artificial cells
    C_iter Ci = cells.begin(), Cj = cells.begin() + 1;            // Artificial target & source cell
    Ci->X = 0;                                                    // Set coordinates of target cell
    Ci->NDLEAF = 10;                                              // Number of leafs in target cell
    Ci->LEAF = ibodies.begin();                                   // Leaf iterator in target cell
    Cj->X = 1;                                                    // Set coordinates of source cell
    Cj->NDLEAF = 1000;                                            // Number of leafs in source cell
    Cj->LEAF = jbodies.begin();                                   // Leaf iterator in source cell
    
    startTimer("M2L kernel   ");                                  // Start timer
    for (int i = 0; i != 1000; ++i) M2L(Ci, Cj);                  // Perform M2L kernel
    timeM2L = stopTimer("M2L kernel   ") / 1000;                  // Stop timer
    startTimer("M2P kernel   ");                                  // Start timer
    for (int i = 0; i != 100; ++i) M2P(Ci, Cj);                   // Perform M2P kernel
    timeM2P = stopTimer("M2P kernel   ") / 1000;                  // Stop timer
    
    Cells icells, jcells;                                         // Artificial cells
    icells.resize(10);                                            // 100 artificial target cells
    jcells.resize(100);                                           // 100 artificial source cells
    Ci0 = icells.begin();                                         // Set global begin iterator for source
    for (Ci = icells.begin(); Ci != icells.end(); ++Ci)    // Loop over target cells
    {
        Ci->X = 0;                                                  //  Set coordinates of target cell
        Ci->NDLEAF = 100;                                           //  Number of leafs in target cell
        Ci->LEAF = ibodies.begin();                                 //  Leaf iterator in target cell
    }                                                             // End loop over target cells
    for (Cj = jcells.begin(); Cj != jcells.end(); ++Cj)    // Loop over source cells
    {
        Cj->X = 1;                                                  //  Set coordinates of source cell
        Cj->NDLEAF = 100;                                           //  Number of leafs in source cell
        Cj->LEAF = jbodies.begin();                                 //  Leaf iterator in source cell
    }                                                             // End loop over source cells
    listP2P.resize(icells.size());                                // Resize P2P interaction list
    flagP2P.resize(icells.size());
    for (Ci = icells.begin(); Ci != icells.end(); ++Ci)    // Loop over target cells
    {
        for (Cj = jcells.begin(); Cj != jcells.end(); ++Cj)  //  Loop over source cells
        {
            listP2P[Ci-Ci0].push_back(Cj);                            //   Push source cell into P2P interaction list
        }                                                           //  End loop over source cells
    }
}

#if QUARK
template<>
void Evaluator<Stokes>::interact(C_iter CI, C_iter CJ, Quark*) {
    PairQueue privateQueue;
    Pair pair(CI,CJ);
    privateQueue.push(pair);
    while( !privateQueue.empty() ) {
        Pair Cij = privateQueue.front();
        privateQueue.pop();
        if(splitFirst(Cij.first,Cij.second)) {
            C_iter C = Cij.first;
            for( C_iter Ci=Ci0+C->CHILD; Ci!=Ci0+C->CHILD+C->NCHILD; ++Ci ) {
                interact(Ci,Cij.second,privateQueue);
            }
        } else {
            C_iter C = Cij.second;
            for( C_iter Cj=Cj0+C->CHILD; Cj!=Cj0+C->CHILD+C->NCHILD; ++Cj ) {
                interact(Cij.first,Cj,privateQueue);
            }
        }
    }
}
#endif

