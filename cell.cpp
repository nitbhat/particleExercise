#include "cell.h"
#include <iostream>
#include <fstream>
#include <string>
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

  int startId = getParticleStartId();

  DEBUG(CmiPrintf("[%d][%d] Populating Cell and start id is %d=======\n", thisIndex.x, thisIndex.y, startId);)
  //along the diagonal
  if(thisIndex.x==thisIndex.y){
    addParticlesOfColor(initialElements,'b', startId);
    addParticlesOfColor(initialElements,'g', startId);
  }

  //lower half-blue particles to be added
  if(thisIndex.x<thisIndex.y)
    addParticlesOfColor(initialElements,'b', startId);

  //upper half-green particles to be added
  if(thisIndex.x>thisIndex.y)
    addParticlesOfColor(initialElements,'g', startId);

  int redBoxMin = (numCellsPerDim-(numCellsPerDim/4))/2;

  //condition for adding red particles
  if(thisIndex.x>=redBoxMin && thisIndex.x<redBoxMin+(numCellsPerDim/4) && thisIndex.y>=redBoxMin && thisIndex.y<redBoxMin +(numCellsPerDim/4))
    addParticlesOfColor(2*initialElements,'r', startId);

  DEBUG(CmiPrintf("[%d][%d] ============================= Added %d particles =======\n", thisIndex.x, thisIndex.y, computeParticlesInCell());)
}

void Cell::addParticlesOfColor(int num, char c, int &startId){
  for(int i=1;i<=num;i++){
    double randomXPosition= startX + custom_drand48()*(cellDim);
    double randomYPosition= startY + custom_drand48()*(cellDim);
    int id = startId + i;

    DEBUG(CmiPrintf("[%d][%d][%d]   [%d][%d] addParticlesOfColor x=%lf, y=%lf, color=%c, gid= %d\n", CmiMyPe(), CmiMyNode(), CmiMyRank(), thisIndex.x, thisIndex.y, randomXPosition, randomYPosition, c, id);)
    Particle p(randomXPosition, randomYPosition, c, id);

    checkParticleBelongsToMe(p);
    particles.push_back(p);
  }
  startId += num; // Update the startId after adding num particles
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

int Cell::computeParticlesInCell(int cellX, int cellY) {
  int numParticles = 0;

  //along the diagonal
  if(cellX==cellY){
    numParticles += 2*(particlesPerCell);
  }

  //lower half-blue or upper half-green
  else if(cellX < cellY || cellX > cellY)
    numParticles += particlesPerCell;

  int redBoxMin = (numCellsPerDim-(numCellsPerDim/4))/2;

  //condition for adding red particles
  if(cellX>=redBoxMin && cellX<redBoxMin+(numCellsPerDim/4) && cellY>=redBoxMin && cellY<redBoxMin +(numCellsPerDim/4))
    numParticles += 2*particlesPerCell;

  return numParticles;

}
int Cell::computeParticlesInCell() {
  return computeParticlesInCell(thisIndex.x, thisIndex.y);
}

int Cell::getParticleStartId() {
  int startId = 0;
  for(int j=0; j <= thisIndex.y; j++) { // iterate over columns
    for(int i=0; i < numCellsPerDim; i++) { // iterate over rows
      if(j == thisIndex.y && i == thisIndex.x)
        return startId;
      startId += computeParticlesInCell(i, j);
    }
  }
  CkAbort("Cell [%d][%d] Couldn't obtain startId for this cell, error!", thisIndex.x, thisIndex.y);
  return -1;
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

void Cell::sortAndDump(string subFolderName) {
  sort(particles.begin(), particles.end());

  // Create a file
  ofstream myFile;

  string myFileName = subFolderName + "/sim_output_" + to_string(thisIndex.x) + "_" + to_string(thisIndex.y);
  myFile.open(myFileName);

  if(myFile.is_open()) {
    myFile << "====================================== BEGIN ==========================================" << endl;
    myFile << "Cell:"<<thisIndex.x <<","<< thisIndex.y<< endl;
    myFile << "=======================================================================================" << endl;
    for(int i=0; i<particles.size(); i++) {
      checkParticleBelongsToMe(particles[i]);
      DEBUG(CmiPrintf("[%d][%d] Final particle Sorted gid=%d => x=%lf, y=%lf, color=%c\n", thisIndex.x, thisIndex.y, particles[i].gid, particles[i].x, particles[i].y, particles[i].color);)
      myFile << "Particle:"<< particles[i].gid <<","<<particles[i].x << "," << particles[i].y << "," << particles[i].color << endl;
    }
    myFile << "====================================== END ==========================================" << endl;
  }
  myFile.close();

  CkCallback doneCb(CkIndex_Main::done(), mainProxy);
  contribute(doneCb);
}

#if LIVEVIZ_RUN
void Cell::mapChareToImage(liveVizRequestMsg *m){
  //CmiPrintf("[%d][%d] Cell::mapChareToImage\n", thisIndex.x, thisIndex.y);
  int beginX = thisIndex.x*(cellDim)*pixelScale;
  int beginY = thisIndex.y*(cellDim)*pixelScale;

  int width = cellDim*pixelScale;
  int height = cellDim*pixelScale;

  unsigned char *imageBuff = new unsigned char[3*width*height];

  //set each pixed to black
  for(int i=0;i<height;++i){
    for(int j=0;j<width;++j){
      imageBuff[3*(i*width+j)+0] = 255;
      imageBuff[3*(i*width+j)+1] = 255;
      imageBuff[3*(i*width+j)+2] = 255;
    }
  }

  for(int i=0;i<particles.size(); i++){

    int xPoint = (particles[i].x - startX)*pixelScale;
    int yPoint = (particles[i].y - startY)*pixelScale;

    if(xPoint>0 && xPoint<width && yPoint>0 && yPoint<height){
      int r=0, g=0, b=0;
      if(particles[i].color=='r') r=255;
      else if(particles[i].color=='g') g=255;
      else if(particles[i].color=='b') b=255;

      int index = yPoint * width + xPoint;

      imageBuff[3*index+0] = r;
      imageBuff[3*index+1] = g;
      imageBuff[3*index+2] = b;
    }
  }

  //set boundaries
  for(int i=0; i<width; ++i){
    imageBuff[3*((height-1)*width+i)+0] = 0;
    imageBuff[3*((height-1)*width+i)+1] = 0;
    imageBuff[3*((height-1)*width+i)+2] = 0;
  }
  for(int i=0;i<height;++i){
    imageBuff[3*(i*width+width-1)+0] = 0;
    imageBuff[3*(i*width+width-1)+1] = 0;
    imageBuff[3*(i*width+width-1)+2] = 0;
  }

  liveVizDeposit (m, beginX, beginY, width, height, imageBuff, this);
  delete [] imageBuff;
}
#endif



