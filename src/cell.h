#ifndef CELL_H
#define CELL_H

#include <stdlib.h>
#include <vector>
#include "pup_stl.h"
#include <string>
#include <assert.h>
using namespace std;
#include "particle.h"

#if LIVEVIZ_RUN
#include "liveViz.h"
#endif

#include "particleSimulation.decl.h"
#include "custom_rand_gen.h"

// This class represent the cells of the simulation.
/// Each cell contains a vector of particle.
// On each time step, the cell perturbs the particles and moves them to neighboring cells as necessary.
class Cell: public CBase_Cell {
  Cell_SDAG_CODE

  public:
    int iteration, numReceived, numParticles, data[3];
    vector<Particle> particles;

    double startX, endX;
    double startY, endY;

    int numOutbound;

    Cell();
    Cell(CkMigrateMessage* m) {}

    void pup(PUP::er &p){
      CBase_Cell::pup(p);
      __sdag_pup(p);
      p | iteration;
      p | particles;
      p | startX;
      p | startY;
      p | endX;
      p | endY;
      p | numOutbound;
    }

    void updateParticles(int iter);
    void updateNeighbor(int iter, std::vector<Particle> incoming, int senderX, int senderY);
    void sortAndDump(string subFolderName);

#if LIVEVIZ_RUN
    void mapChareToImage(liveVizRequestMsg *m);
#endif

#if BONUS_QUESTION
    void contributeToReduction();
#endif

  private:
    void populateCell(int initialElements);
    void perturb(Particle* particle);
    void addParticlesOfColor(int num, char c, int &startId);

    void reduceTotalAndMax();

    void sendParticles(int xIndex, int yIndex, int iteration,  std::vector<Particle> &outgoing) {
      numOutbound += outgoing.size();
      thisProxy(xIndex, yIndex).receiveUpdate(iteration, outgoing, thisIndex.x, thisIndex.y);
    }

    void checkParticleBelongsToMe(Particle &p) {
        // Error checking
        if(p.x < startX || p.x > endX)
          CmiAbort("[%d][%d] Particle X coordinate %lf doesn't belong in [%lf, %lf]\n", thisIndex.x, thisIndex.y, p.x, startX, endX);

        else if(p.y < startY || p.y > endY)
          CmiAbort("[%d][%d] Particle Y coordinate %lf doesn't belong in [%lf, %lf]\n", thisIndex.x, thisIndex.y, p.y, startY, endY);
    }

    int computeParticlesInCell(int cellX, int cellY);
    int computeParticlesInCell();
    int getParticleStartId();

};

#endif
