#ifndef MAIN_H
#define MAIN_H
#include <string>
using namespace std;

#if LIVEVIZ_RUN
#include "liveViz.h"
#endif

#include "particleSimulation.decl.h"
#include "custom_rand_gen.h"

#define PIXEL_SCALE (8)


class Main: public CBase_Main {

  CProxy_Cell cellGrid;
  double startTime, endTime, totalTime;

  int counter, total;

  public:
    Main(CkArgMsg* m);

    //function to receive the reduction result
    void receiveReductionData(CkReductionMsg *data);
    void done();
    void printTotal(int total, int max, int iter);

    void readyToOutput();

#if BONUS_QUESTION
    void computeMin(int min);
    void computeMax(int max);
#endif
};

#endif
