#ifndef MAIN_H
#define MAIN_H
#include <string>
using namespace std;

#if LIVEVIZ_RUN
#include "liveViz.h"
#endif

#include "particleSimulation.decl.h"
#include "custom_rand_gen.h"

#define ITERATION (100)
#define LBFREQ (10)
#define PIXEL_SCALE (8)

/*readonly*/ CProxy_Main mainProxy;
/*readonly*/ CProxy_Cell cellProxy;
/*readonly*/ int particlesPerCell;
/*readonly*/ int numCellsPerDim;
/*readonly*/ double boxMax;
/*readonly*/ double boxMin;
/*readonly*/ double cellDim;

#if LIVEVIZ_RUN
/*readonly*/ double pixelScale;
#endif

CkReduction::reducerType totalAndMaxType;

#endif
