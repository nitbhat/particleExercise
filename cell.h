#ifndef CELL_H
#define CELL_H

#include <stdlib.h>
#include <vector>
#include "pup_stl.h"
#include "particle.h"
#include "particleExercise.decl.h"

using namespace std;
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

      custom_srand48(thisIndex.x + (numCellsPerDim)*thisIndex.y);
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

    //int wrap(int index);

    void reduceTotalAndMax(){
      numParticles=particles.size();
      data[0]=numParticles;
      data[1]=numParticles;
      data[2]=iteration;
      CkCallback cbTotalAndMax(CkIndex_Main::receiveReductionData(NULL),mainProxy);
      contribute(3*sizeof(int), data, totalAndMaxType, cbTotalAndMax);
    }

    void sendParticles(int xIndex, int yIndex, int iteration,  std::vector<Particle> &outgoing) {
      thisProxy(xIndex, yIndex).receiveUpdate(iteration, outgoing);
    }

};

#endif
