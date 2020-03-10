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

#include "cell.h"
#include "main.h"

// Useful function declarations
//void Cell::perturb(Particle* particle);
//void Cell::sendParticles(int xIndex, int yIndex, int iteration,  std::vector<Particle> &outgoing);


//change the position of the particles and send messages to neighbors with their incoming particles
void Cell::updateParticles(int iter) {

  //TODO: Add code for the following
  // 1. Iterate through the particles and call perturb(...) passing each particle. This causes the particle's x and y coordinate to change
  // 2. Identify the new cell that the particle belongs to and construct a vector of particles to be sent to each of the 8 neighbors (topLeft, top, topRight, left, right, bottomLeft, bottom, bottomRight)
  // 3. Make sure that particles that go outside the bounding box are wrapped back i.e for a 2D box consisting of 8 cells in each dimension,
  //    if a particle with the index (7,7) goes to (7, 8), it should be sent back to (7, 0).
  // 3. Call sendParticles(...) to send the 8 different vector of particles to each of the 8 neighbours

}

#if BONUS_QUESTION
void Main::receiveMinMaxReductionData(CkReductionMsg *data) {
  int *output = (int *) data->getData();

  // TODO: Assign values to maxParticles, maxCellX, maxCellY, minParticles, minCellX, minCellY based
  // on computed reduction values

  CmiPrintf("Max Particles:%d, Cell with Max Particles: (%d, %d)\n", maxParticles, maxCellX, maxCellY);
  CmiPrintf("Min Particles:%d, Cell with Min Particles: (%d, %d)\n", minParticles, minCellX, minCellY);
  readyToOutput();
}

void Cell::contributeToReduction() {
  numParticles = particles.size();

  //TODO: Add code to contribute to reduction for finding the different custom reduction values
  // Declare a callback with the function receiveMinMaxReductionData
}

CkReductionMsg *calculateMaxMin(int nMsg, CkReductionMsg **msgs) {
  //TODO: Add code to compute the following:
  // Value of the maximum particles per cell
  // X coordinate of the cell with the maximum particles per cell
  // Y coordinate of the cell with the maximum particles per cell
  // Value of the minimum particles per cell
  // X coordinate of the cell with the minimum particles per cell
  // Y coordinate of the cell with the minimum particles per cell
}


void Cell::contributeToReduction() {
  //TODO: Add code to contribute to reduction for finding the minimum value of the number of particles
  //TODO: Add code to contribute to reduction for finding the maximum value of the number of particles
}
#endif
