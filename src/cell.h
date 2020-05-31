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

    // vector of my particles
    vector<Particle> particles;

    // startX is my cell's starting X coordinate
    // endX is my cell's ending X coordinate
    double startX, endX;

    // startY is my cell's starting Y coordinate
    // endY is my cell's ending Y coordinate
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
      p | myShare;
      p | ppcEqualDist;
    }

    void updateParticles(int iter);
    void updateNeighbor(int iter, std::vector<Particle> incoming, int senderX, int senderY);
    void sortAndDump(string subFolderName);
    void reorganizeParticles(string subFolderName);
    void recvParticlesPostSimulation(vector<Particle> inbound);

    void verifyCorrectness();

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

    void sendParticlesPostSimulation(int linearCellId, vector<Particle> &outbound);

    void checkParticleBelongsToMe(Particle &p) {
        // Error checking
        if(p.x < startX - 1e-8 || p.x > endX + 1e-8)
          CmiAbort("[%d][%d] Particle X coordinate %lf doesn't belong in [%lf, %lf]\n", thisIndex.x, thisIndex.y, p.x, startX, endX);

        else if(p.y < startY - 1e-8 || p.y > endY + 1e-8)
          CmiAbort("[%d][%d] Particle Y coordinate %lf doesn't belong in [%lf, %lf]\n", thisIndex.x, thisIndex.y, p.y, startY, endY);
    }

    int totalParticles;
    int myShare;
    int ppcEqualDist;
    // vector of my particles after reorganization
    vector<Particle> reorgParticles;

    // vector of particles read in from pre-computed output file
    // These particles are compared against simulation particles to verify correctness
    vector<Particle> precomputeParticles;

    string outputFolderName;

    void computeTotalParticles();

    int computeParticlesInCell(int cellX, int cellY);
    int computeParticlesInCell();
    int getParticleStartId();
    void readComparisonOutputFromFiles();
};

#endif
