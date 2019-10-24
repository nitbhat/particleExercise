#include "cell.h"

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
