#include <string>
using namespace std;

#if LIVEVIZ_RUN
#include "liveViz.h"
#endif

#include "particleSimulation.decl.h"
#include "custom_rand_gen.h"
/*readonly*/ extern CProxy_Main mainProxy;
/*readonly*/ extern CProxy_Cell cellProxy;
/*readonly*/ extern int particlesPerCell;
/*readonly*/ extern int numCellsPerDim;
/*readonly*/ extern int iterations;
/*readonly*/ extern int lbFreq;
/*readonly*/ extern int reductionFreq;
/*readonly*/ extern double boxMax;
/*readonly*/ extern double boxMin;
/*readonly*/ extern double cellDim;

extern CkReduction::reducerType minMaxType;

#include "cell.h"
#include "main.h"
#define DEBUG(x) //x

int wrap(int index) {
  if(index == -1) return numCellsPerDim - 1;
  else if(index == numCellsPerDim) return 0;
  else return index;
}

void Cell::updateParticles(int iter) {

  //declaring vectors for particles whose position needs to be changed
  vector<Particle> topLeft, top, topRight, left, right, bottomLeft, bottom, bottomRight;
  int newXIndex, newYIndex;

  // Iterate over the particles and call 'perturb'
  int swapIndex = particles.size()-1;

  DEBUG(CmiPrintf("[%d][%d] ============================= update particles beginning ITER: %d=======\n", thisIndex.x, thisIndex.y, iter);)

  for(int i=particles.size()-1;i>=0; i--){

    //change the position of the particle
    perturb(&particles[i]);

    double xPos = particles[i].x;
    double yPos = particles[i].y;

    //find the cell the particle belongs
    //condition for shifting to top-left cell
    if(xPos < startX && yPos < startY){
      topLeft.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to top cell
    else if(xPos > startX && xPos < endX && yPos < startY){
      top.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to top-right cell
    else if(xPos > endX && yPos < startY){
      topRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to left cell
    else if(xPos < startX && yPos > startY && yPos < endY){
      left.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to right cell
    else if(xPos > endX && yPos > startY && yPos < endY) {
      right.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-left cell
    else if(xPos < startX && yPos > endY){
      bottomLeft.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom cell
    else if(xPos > startX && xPos < endX && yPos > endY){
      bottom.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-right cell
    else if(xPos > endX && yPos > endY){
      bottomRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }
  }

  DEBUG(CmiPrintf("[%d][%d] ============================= update particles end ITER: %d=======\n", thisIndex.x, thisIndex.y, iter);)
  particles.resize(swapIndex+1);

  int myX = thisIndex.x;
  int myY = thisIndex.y;

  // Send particles to my top row
  sendParticles(wrap(myX - 1), wrap(myY - 1), iter, topLeft);
  sendParticles(myX    , wrap(myY - 1), iter, top);
  sendParticles(wrap(myX + 1), wrap(myY - 1), iter, topRight);

  // Send particles to my row (left and right)
  sendParticles(wrap(myX - 1), myY    , iter, left);
  sendParticles(wrap(myX + 1), myY    , iter, right);

  // Send particles to my bottom row
  sendParticles(wrap(myX - 1), wrap(myY + 1), iter, bottomLeft);
  sendParticles(myX    , wrap(myY + 1), iter, bottom);
  sendParticles(wrap(myX + 1), wrap(myY + 1), iter, bottomRight);
}

#if BONUS_QUESTION
void Main::receiveMinMaxReductionData(CkReductionMsg *data) {
  int *output = (int *) data->getData();

  maxParticles = output[0];
  maxCellX = output[1];
  maxCellY = output[2];

  minParticles = output[3];
  minCellX = output[4];
  minCellY = output[5];

  CmiPrintf("Max Particles:%d, Cell with Max Particles: (%d, %d)\n", maxParticles, maxCellX, maxCellY);
  CmiPrintf("Min Particles:%d, Cell with Min Particles: (%d, %d)\n", minParticles, minCellX, minCellY);
  readyToOutput();
}

void Cell::contributeToReduction() {
  numParticles = particles.size();

  int minMaxData[6];

  minMaxData[0] = numParticles;
  minMaxData[1] = thisIndex.x;
  minMaxData[2] = thisIndex.y;

  minMaxData[3] = numParticles;
  minMaxData[4] = thisIndex.x;
  minMaxData[5] = thisIndex.y;

  CkCallback cbMinMax(CkIndex_Main::receiveMinMaxReductionData(NULL), mainProxy);
  contribute(6*sizeof(int), minMaxData, minMaxType, cbMinMax);
}

CkReductionMsg *calculateMaxMin(int nMsg, CkReductionMsg **msgs) {

  int returnVal[6];

  // signifies max value
  returnVal[0] = -1;

  // signifies x coordinate of the cell with max value
  returnVal[1] = -1;

  // signifies y coordinate of the cell with max value
  returnVal[2] = -1;

  // signifies min value
  returnVal[3] = INT_MAX;

  // signifies x coordinate of the cell with min value
  returnVal[4] = -1;

  // signifies y coordinate of the cell with min value
  returnVal[5] = -1;

  for (int i=0;i<nMsg;i++) {
    CkAssert(msgs[i]->getSize()==6*sizeof(int));
    int *m = (int *)msgs[i]->getData();

    if(m[0] > returnVal[0]) {
      returnVal[0] = m[0]; // Set the new max
      returnVal[1] = m[1];
      returnVal[2] = m[2];
    }

    if(m[3] < returnVal[3]) {
      returnVal[3] = m[3]; // Set the new min
      returnVal[4] = m[4];
      returnVal[5] = m[5];
    }
  }
  return CkReductionMsg::buildNew(6*sizeof(int),returnVal);
}
#endif
