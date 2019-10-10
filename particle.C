#include "particleExercise.decl.h"

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
CkReduction::reducerType totalAndMaxType;
#include "cell.h"

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
      CkExit();
    }
  }

  // and max counts and exiting when the iterations are done
  void printTotal(int total, int max, int iter){
    CkPrintf("ITER %d, MAX: %d, TOTAL: %d\n", iter, max, total);
  }
};

#include "cell.h"

// Other particle methods
void Cell::populateCell(int initialElements) {

  //along the diagonal
  if(thisIndex.x==thisIndex.y){
    addParticlesOfColor(initialElements,'b');
    addParticlesOfColor(initialElements,'g');
  }

  //lower half-blue particles to be added
  if(thisIndex.x<thisIndex.y)
    addParticlesOfColor(initialElements,'b');

  //upper half-green particles to be added
  if(thisIndex.x>thisIndex.y)
    addParticlesOfColor(initialElements,'g');

  int redBoxMin = (numCellsPerDim-(numCellsPerDim/4))/2;

  //condition for adding red particles
  if(thisIndex.x>=redBoxMin && thisIndex.x<redBoxMin+(numCellsPerDim/4) && thisIndex.y>=redBoxMin && thisIndex.y<redBoxMin +(numCellsPerDim/4))
    addParticlesOfColor(2*initialElements,'r');
}

void Cell::addParticlesOfColor(int num, char c){
  for(int i=1;i<=num;i++){
    double randomXPosition= startX + drand48()*(cellDim);
    double randomYPosition= startY + drand48()*(cellDim);
    //CmiPrintf("[%d][%d][%d]   [%d][%d] addParticlesOfColor x=%lf, y=%lf\n", CmiMyPe(), CmiMyNode(), CmiMyRank(), thisIndex.x, thisIndex.y, randomXPosition, randomYPosition);
    Particle p(randomXPosition, randomYPosition, c);
    particles.push_back(p);
  }
}

//change the location of the particle within the range of 8 neighbours
//the location of the particles might exceed the bounds of the chare array
//as a result of this functions, so you need to handle that case when deciding
//which particle to go which neighbour chare
//e.g. the right neighbour of chare indexed[k-1,0] is chare [0,0]
void Cell::perturb(Particle* particle) {
  //drand48 creates a random number between [0-1]
  double deltax = (drand48()-drand48())*10;
  double deltay = (drand48()-drand48())*10;

  if(particle->color=='r'){
    particle->x += deltax;
    particle->y += deltay;
  }
  else if(particle->color=='b'){
    particle->x += deltax/2;
    particle->y += deltay/2;
  }
  else if(particle->color=='g'){
    particle->x += deltax/4;
    particle->y += deltay/4;
  }
}

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

#include "particleExercise.def.h"
