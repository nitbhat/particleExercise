#include <stdlib.h>
#include <vector>
#include "pup_stl.h"
#include "Particle.h"
#include "ParticleExercise.decl.h"

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

using namespace std;

CkReduction::reducerType totalAndMaxType;

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

    //declare box dimensions
    boxMax=100.0;
    boxMin=0.0;

    cellDim = boxMax/numCellsPerDim;

    CmiPrintf("[%d][%d][%d] particlesPerCell = %d\n", CmiMyPe(), CmiMyNode(), CmiMyRank(), particlesPerCell);
    CmiPrintf("[%d][%d][%d] numCellsPerDim = %d\n", CmiMyPe(), CmiMyNode(), CmiMyRank(), numCellsPerDim);

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
      CkExit();
    }
  }

  // and max counts and exiting when the iterations are done
  void printTotal(int total, int max, int iter){
    CkPrintf("ITER %d, MAX: %d, TOTAL: %d\n", iter, max, total);
  }
};

// This class represent the cells of the simulation.
/// Each cell contains a vector of particle.
// On each time step, the cell perturbs the particles and moves them to neighboring cells as necessary.
class Cell: public CBase_Cell {
  Cell_SDAG_CODE

  public:
    int iteration, numReceived, numParticles, data[3];
    vector<Particle> particles;

    double startX;
    double startY;

    Cell() {
      __sdag_init();
      iteration = 0;
      usesAtSync = true;
      startX = (double) thisIndex.x*(cellDim);
      startY = (double) thisIndex.y*(cellDim);

      srand48(thisIndex.x + (numCellsPerDim)*thisIndex.y);
      populateCell(particlesPerCell); //creates random particles within the cell
    }
    Cell(CkMigrateMessage* m) {}

    void pup(PUP::er &p){
      CBase_Cell::pup(p);
      __sdag_pup(p);
      p|iteration;
      p|particles;
      p | startX;
      p | startY;
    }

    void updateNeighbor(int iter, std::vector<Particle> incoming){
      particles.insert(particles.end(), incoming.begin(), incoming.end());
    }

    void updateParticles(int iter);

  private:
    void populateCell(int initialElements);
    void perturb(Particle* particle);
    void addParticlesOfColor(int num, char c);

    void reduceTotalAndMax(){
      numParticles=particles.size();
      data[0]=numParticles;
      data[1]=numParticles;
      data[2]=iteration;
      CkCallback cbTotalAndMax(CkIndex_Main::receiveReductionData(NULL),mainProxy);
      contribute(3*sizeof(int), data, totalAndMaxType, cbTotalAndMax);
    }

};

CkReductionMsg *calculateTotalAndMax(int nMsg, CkReductionMsg **msgs)
{
  int returnVal[3];

  //signifies total value
  returnVal[0]=0;

  //signifies maximum value
  returnVal[1]=0;

  for (int i=0;i<nMsg;i++) {
    CkAssert(msgs[i]->getSize()==3*sizeof(int));
    int *m=(int *)msgs[i]->getData();

    returnVal[0]+=m[0];

    if(m[1]>returnVal[1])
      returnVal[1]=m[1];

    returnVal[2]=m[2];
  }
  return CkReductionMsg::buildNew(3*sizeof(int),returnVal);
}

void registerCalculateTotalAndMax(void){
  totalAndMaxType=CkReduction::addReducer(calculateTotalAndMax);
}

#include "preset.C"
#include "exercise.C"
#include "ParticleExercise.def.h"
