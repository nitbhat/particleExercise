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
/*readonly*/ int cellDimension;
/*readonly*/ double boxMax;
/*readonly*/ double boxMin;

using namespace std;

CkReduction::reducerType totalAndMaxType;

class Main: public CBase_Main {

  CProxy_Cell cellGrid;
  public:
    Main(CkArgMsg* m) {
      if(m->argc < 3) CkAbort("USAGE: ./charmrun +p<number_of_processors> ./particle <number of particles per cell> <size of array>");

      mainProxy = thisProxy;
      particlesPerCell = atoi(m->argv[1]);
      cellDimension = atoi(m->argv[2]);
      delete m;

      //declare box dimensions
      boxMax=100.0;
      boxMin=0.0;

      //declare a 2D chare array with dimensions cellDimension*cellDimension
      CkArrayOptions opts(cellDimension, cellDimension);
      cellGrid = CProxy_Cell::ckNew(opts);

      //start the run for all the chares
      cellGrid.run();
    }

    //function to receive the reduction result
    void receiveReductionData(CkReductionMsg *data){
      int *output = (int *) data->getData();
      printTotal(output[0], output[1], output[2]);
      if(output[2] == ITERATION)
        CkExit();
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
      startX = (double) thisIndex.x*(boxMax/cellDimension);
      startY = (double) thisIndex.y*(boxMax/cellDimension);
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

    //change the position of the particles and send messages to neighbors with their incoming particles
    void updateParticles(int iter) {
      //declaring vectors for particles whose position needs to be changed
      std::vector<Particle> topLeft, top, topRight, left, right, bottomLeft, bottom, bottomRight;
      int newXIndex, newYIndex;

      int swapIndex = particles.size()-1;
      for(int i=particles.size()-1;i>=0; i--){

        //change the position of the particle
        perturb(&particles[i]);

        //find the cell the particle belongs
        //condition for shifting to top-left cell
        if(particles[i].x<startX && particles[i].y<startY){
          topLeft.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to top cell
        else if(particles[i].x>startX && particles[i].x<startX + (boxMax/cellDimension) && particles[i].y<startY){
          top.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to top-right cell
        else if(particles[i].x>startX+(boxMax/cellDimension) && particles[i].y<startY){
          topRight.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to left cell
        else if(particles[i].x < startX && particles[i].y>startY && particles[i].y<startY+ (boxMax/cellDimension)){
          left.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to right cell
        else if(particles[i].x>startX+(boxMax/cellDimension) && particles[i].y>startY && particles[i].y<startY + (boxMax/cellDimension))
        {
          right.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to bottom-left cell
        else if(particles[i].x<startX && particles[i].y>startY+(boxMax/cellDimension)){
          bottomLeft.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to bottom cell
        else if(particles[i].x>startX && particles[i].x<startX+(boxMax/cellDimension) && particles[i].y>startY+ (boxMax/cellDimension)){
          bottom.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }

        //condition for shifting to bottom-right cell
        else if(particles[i].x >startX+ (boxMax/cellDimension) && particles[i].y>startY+ (boxMax/cellDimension)){
          bottomRight.push_back(particles[i]);
          swap(particles[i], particles[swapIndex--]);
        }
      }
      particles.resize(swapIndex+1);

      //shifting the particles to the correct cells
      newXIndex = thisIndex.x-1;
      newYIndex = thisIndex.y-1;
      if(newXIndex==-1) newXIndex=cellDimension-1;
      if(newYIndex==-1) newYIndex=cellDimension-1;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,topLeft);

      newXIndex = thisIndex.x;
      newYIndex = thisIndex.y-1;
      if(newYIndex==-1) newYIndex=cellDimension-1;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,top);

      newXIndex = thisIndex.x+1;
      newYIndex = thisIndex.y-1;
      if(newXIndex==cellDimension) newXIndex=0;
      if(newYIndex==-1) newYIndex=cellDimension-1;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,topRight);

      newXIndex = thisIndex.x-1;
      newYIndex = thisIndex.y;
      if(newXIndex==-1) newXIndex=cellDimension-1;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,left);

      newXIndex = thisIndex.x+1;
      newYIndex = thisIndex.y;
      if(newXIndex==cellDimension) newXIndex=0;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,right);

      newXIndex = thisIndex.x-1;
      newYIndex = thisIndex.y+1;
      if(newXIndex==-1) newXIndex=cellDimension-1;
      if(newYIndex==cellDimension) newYIndex=0;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottomLeft);

      newXIndex = thisIndex.x;
      newYIndex = thisIndex.y+1;
      if(newYIndex==cellDimension) newYIndex=0;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottom);

      newXIndex = thisIndex.x+1;
      newYIndex = thisIndex.y+1;
      if(newXIndex==cellDimension) newXIndex=0;
      if(newYIndex==cellDimension) newYIndex=0;
      thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottomRight);

    }

  private:
   void populateCell(int initialElements) {
    //create random particles and add then to the particles vector

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

    int redBoxMin = (cellDimension-(cellDimension/4))/2;

    //condition for adding red particles
    if(thisIndex.x>=redBoxMin && thisIndex.x<redBoxMin+(cellDimension/4) && thisIndex.y>=redBoxMin && thisIndex.y<redBoxMin +(cellDimension/4))
      addParticlesOfColor(2*initialElements,'r');

   }

   void addParticlesOfColor(int num, char c){
    for(int i=1;i<=num;i++){
      double randomXPosition= startX + drand48()*(boxMax/cellDimension);
      double randomYPosition= startY + drand48()*(boxMax/cellDimension);
      Particle p(randomXPosition, randomYPosition, c);
      particles.push_back(p);
    }
   }

    //change the location of the particle within the range of 8 neighbours
    //the location of the particles might exceed the bounds of the chare array
    //as a result of this functions, so you need to handle that case when deciding
    //which particle to go which neighbour chare
    //e.g. the right neighbour of chare indexed[k-1,0] is chare [0,0]
    void perturb(Particle* particle) {
      //drand48 creates a random number between [0-1]
      double deltax = (drand48()-drand48())*10;
      double deltay = (drand48()-drand48())*10;

      if(particle->color=='r'){
        particle->x += deltax;
        particle->y += deltay;
      }
      else if(particle->color=='b'){
        particle->x += deltax/2;
        particle->y += deltax/2;
      }
      else if(particle->color=='g'){
        particle->x += deltax/4;
        particle->x += deltax/4;
      }
    }

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

#include "ParticleExercise.def.h"
