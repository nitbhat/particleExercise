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
    double randomXPosition= startX + custom_drand48()*(cellDim);
    double randomYPosition= startY + custom_drand48()*(cellDim);
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
  //custom_drand48 creates a random number between [0-1]
  //double deltax = (custom_drand48()-custom_drand48())*10;
  //double deltay = (custom_drand48()-custom_drand48())*10;

  if(particle->color=='r'){
    particle->x = particle->x; // don't modify x coordinate
    particle->y += 0.3;         // moves up by 0.3
  }
  else if(particle->color=='b'){
    particle->x -= 0.1;         // moves left by 0.1
    particle->y -= 0.2;         // moves down by 0.2
  }
  else if(particle->color=='g'){
    particle->x += 0.2;         // moves right by 0.2
    particle->y += 0.1;         // moves top by 0.1
  }
}

void Cell::reduceTotalAndMax() {
  numParticles=particles.size();
  data[0]=numParticles;
  data[1]= numOutbound;
  data[2]=iteration;
  CkCallback cbTotalAndMax(CkIndex_Main::receiveReductionData(NULL),mainProxy);

  // Reset numOutbound value to 0 for the next iteration
  numOutbound = 0;

  contribute(3*sizeof(int), data, totalAndMaxType, cbTotalAndMax);
}




