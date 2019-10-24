#include "cell.h"
#define DEBUG(x) //x

Cell::Cell() {
  DEBUG(CmiPrintf("[%d][%d] ******************** Constructor *********************\n", thisIndex.x, thisIndex.y);)
  __sdag_init();
  iteration = 0;
  numOutbound = 0;
  usesAtSync = true;
  startX = (double) thisIndex.x*(cellDim);
  startY = (double) thisIndex.y*(cellDim);

  endX = startX + cellDim;
  endY = startY + cellDim;

  custom_srand48(thisIndex.x + (numCellsPerDim)*thisIndex.y);

  DEBUG(CmiPrintf("[%d][%d] ============================= Populating Cell=======\n", thisIndex.x, thisIndex.y);)
  populateCell(particlesPerCell); //creates random particles within the cell
  DEBUG(CmiPrintf("[%d][%d] ============================= Done Populating Cell=======\n", thisIndex.x, thisIndex.y);)
}


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

    checkParticleBelongsToMe(p);
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


void Cell::updateNeighbor(int iter, std::vector<Particle> incoming, int senderX, int senderY){

  DEBUG(CmiPrintf("[%d][%d] ============================= update neighbor beginning ITER: %d coming in from [%d][%d] =======\n", thisIndex.x, thisIndex.y, iter, senderX, senderY);)

  for(int i=0; i<incoming.size(); i++) {

    if(thisIndex.y == 0) { // Top boundary cell

      if(incoming[i].y > boxMax) // reset position
        incoming[i].y = incoming[i].y - boxMax;

    } else if(thisIndex.y == numCellsPerDim - 1) { // Bottom boundary cell

      if(incoming[i].y < boxMin) //reset position
        incoming[i].y = boxMax + incoming[i].y;

    }

    if(thisIndex.x == 0) { // Left boundary cell

      if(incoming[i].x > boxMax) // reset position
        incoming[i].x = incoming[i].x - boxMax;

    } else if(thisIndex.x == numCellsPerDim - 1) { // Right boundary cell

      if(incoming[i].x < boxMin) // reset position
        incoming[i].x = boxMax + incoming[i].x;

    }
    checkParticleBelongsToMe(incoming[i]);
  }

  DEBUG(CmiPrintf("[%d][%d] ============================= update neighbor end ITER: %d=======\n", thisIndex.x, thisIndex.y, iter);)
  particles.insert(particles.end(), incoming.begin(), incoming.end());
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




