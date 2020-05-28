#ifndef MAIN_H
#define MAIN_H
#include <string>
#include <assert.h>
using namespace std;

#if LIVEVIZ_RUN
#include "liveViz.h"
#endif

#include "particleSimulation.decl.h"
#include "custom_rand_gen.h"

#define PIXEL_SCALE (8)


class Main: public CBase_Main {

  double startTime, endTime, totalTime;

  int minParticles, maxParticles;
  int minCellX, minCellY;
  int maxCellX, maxCellY;

  bool outputPrompt;
  string finalPath;

  int particleContributionType;

  CkReductionMsg *simulationParticlesMsg;
  CkReductionMsg *verifiedSimulationParticlesMsg;

  public:
    Main(CkArgMsg* m);

    //function to receive the reduction result
    void receiveTotalOutboundReductionData(CkReductionMsg *data);
    void done();
    void printTotal(int total, int max, int iter);

    void readyToOutput();
    bool getUserInput();
    string getDefaultSubdirectoryName();

    void getAllParticles(CkReductionMsg *msg);
    void verifyOutput();

#if BONUS_QUESTION
    void computeMin(int min);
    void computeMax(int max);
    void receiveMinMaxReductionData(CkReductionMsg *data);
#endif
};

#endif
