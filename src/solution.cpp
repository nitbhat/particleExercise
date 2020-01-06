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
/*readonly*/ extern double boxMax;
/*readonly*/ extern double boxMin;
/*readonly*/ extern double cellDim;
extern CkReduction::reducerType totalAndMaxType;
#include "cell.h"
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
