#include "main.h"
#include "cell.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>

class Main: public CBase_Main {

  CProxy_Cell cellGrid;

  double startTime, endTime, totalTime;

  public:
  Main(CkArgMsg* m) {
    if(m->argc < 3) CkAbort("USAGE: ./charmrun +p<number_of_processors> ./particle <number of particles per cell> <size of array>");

    mainProxy = thisProxy;
    particlesPerCell = atoi(m->argv[1]);
    numCellsPerDim = atoi(m->argv[2]);
    delete m;

    // Each cell has 1.0 * 1.0 dimensions and hence the total box dimensions will be 0.0 and 1.0 * numCellsPerDim
    // declare box dimensions
    boxMax = numCellsPerDim * 1.0;
    boxMin = 0.0;

    cellDim = 1.0;

    CmiPrintf("NumCellsPerDim = %d\n", numCellsPerDim);

    //declare a 2D chare array with dimensions numCellsPerDim*numCellsPerDim
    CkArrayOptions opts(numCellsPerDim, numCellsPerDim);
    cellGrid = CProxy_Cell::ckNew(opts);

    startTime = CkWallTimer();

    //start the run for all the chares
    cellGrid.run();
  }

  //function to receive the reduction result
  void receiveReductionData(CkReductionMsg *data){
    int *output = (int *) data->getData();
    //CkAssert(output[2] == particlesPerCell*numCellsPerDim*numCellsPerDim);
    printTotal(output[0], output[1], output[2]);
    if(output[2] == ITERATION) {
      endTime = CkWallTimer();
      totalTime = (endTime - startTime);
      CkPrintf("Simulation Complete, total time taken is %lf seconds\n", totalTime);

      struct stat info;
      if(stat("output", &info) != 0) {
        // Create an output directory
        const int mkdirOut = mkdir("output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (-1 == mkdirOut) {
          CmiAbort("Error while creating the output directory");
        }
      }

      char runOutputFolder[80];
      time_t t = time(0);
      struct tm *now = localtime(&t);
      strftime(runOutputFolder, 80, "sim_output_%Y-%m-%d-%H-%M-%S", now);

      string name(runOutputFolder);
      string parentFolder("output/");
      string finalPath = parentFolder + name;

      // Create an output subdirectory that is dependent on the current time
      const int mkdirOut = mkdir(finalPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if (-1 == mkdirOut) {
        CmiAbort("Error while creating the output sub-directory");
      }

      // Make each chare write to a file
      cellGrid.sortAndDump(finalPath);
    }
  }

  void done() {
    CkPrintf("All output has been written to files, Exiting program\n");
    CkExit();
  }

  // and max counts and exiting when the iterations are done
  void printTotal(int total, int max, int iter){
    CkPrintf("Iteration: %d, Outgoing Particles Sum: %d, Total Particles: %d\n", iter, max, total);
  }
};


// Global Functions
CkReductionMsg *calculateTotalAndMax(int nMsg, CkReductionMsg **msgs) {
  int returnVal[3];

  //signifies total particles sum value
  returnVal[0]=0;

  //signifies outgoing particles sum value
  returnVal[1]=0;

  for (int i=0;i<nMsg;i++) {
    CkAssert(msgs[i]->getSize()==3*sizeof(int));
    int *m=(int *)msgs[i]->getData();

    returnVal[0]+=m[0]; // Sum of total particles

    returnVal[1]+=m[1]; // Sum of outbound particles

    returnVal[2]=m[2];
  }
  return CkReductionMsg::buildNew(3*sizeof(int),returnVal);
}

void registerCalculateTotalAndMax(void){
  totalAndMaxType=CkReduction::addReducer(calculateTotalAndMax);
}

#include "particleSimulation.def.h"
