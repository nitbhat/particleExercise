#include "cell.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <iomanip> //for set precision
#define DEBUG(x) //x

extern CProxy_Main mainProxy;
extern CProxy_Cell cellProxy;
extern int particlesPerCell;
extern int numCellsPerDim;
extern double boxMax;
extern double boxMin;
extern double cellDim;
extern int velocityFactor;
extern vector<int> particleRatio;
extern bool logOutput;


#if LIVEVIZ_RUN
extern double pixelScale;
#endif

extern CkReduction::reducerType totalOutboundType;

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

  computeTotalParticles();

  // Code used for reorganization of particles after simulation
  ppcEqualDist = totalParticles/(numCellsPerDim * numCellsPerDim);
  // All cells except the last cell will have ppcEqualDist particles as their share
  if(thisIndex.x == (numCellsPerDim - 1) &&  thisIndex.y == (numCellsPerDim - 1)) {
    myShare = totalParticles - ppcEqualDist*(numCellsPerDim*numCellsPerDim - 1);
  } else {
    myShare = ppcEqualDist;
  }
}

// Other particle methods
void Cell::populateCell(int initialElements) {

  int startId = getParticleStartId();

  DEBUG(CmiPrintf("[%d][%d] Populating Cell and start id is %d=======\n", thisIndex.x, thisIndex.y, startId);)

  //lower half-green particles to be added
  if(thisIndex.x < thisIndex.y)
    addParticlesOfColor(particleRatio[0] * initialElements,'g', startId);

  //upper half-blue particles to be added
  if(thisIndex.x > thisIndex.y)
    addParticlesOfColor(particleRatio[1] * initialElements,'b', startId);

  //along the diagonal
  if(thisIndex.x == thisIndex.y){
    addParticlesOfColor(particleRatio[2] * initialElements,'r', startId);
  }

  int redBoxMin = (numCellsPerDim-(numCellsPerDim/4))/2;

  //condition for adding red particles
  if(thisIndex.x >= redBoxMin && thisIndex.x < redBoxMin+(numCellsPerDim/4) && thisIndex.y >= redBoxMin && thisIndex.y < redBoxMin +(numCellsPerDim/4))
    addParticlesOfColor(particleRatio[3] * initialElements,'r', startId);

  DEBUG(CmiPrintf("[%d][%d] ============== Added %d particles and end id is %d =======\n", thisIndex.x, thisIndex.y, computeParticlesInCell(), startId);)
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

  //CmiPrintf("[%d][%d] deltax, deltay [%lf, %lf]\n", thisIndex.x, thisIndex.y, deltax, deltay);

  double deltax = cos(particle->y);
  double deltay = cos(particle->x);
  assert(deltax >= -1 && deltax <= 1);
  assert(deltay >= -1 && deltay <= 1);

  if(particle->color=='r'){
    particle->x += deltax/velocityFactor; // don't modify x coordinate
    particle->y += deltay/velocityFactor;         // moves up by 0.3
  }
  else if(particle->color=='b'){
    particle->x += deltax/(velocityFactor * 2);         // moves left by 0.1
    particle->y += deltay/(velocityFactor * 2);         // moves down by 0.2
  }
  else if(particle->color=='g'){
    particle->x += deltax/(velocityFactor * 5);         // moves right by 0.2
    particle->y += deltay/(velocityFactor * 5);         // moves top by 0.1
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

  //lower half-green particles to be added
  if(cellX < cellY)
    numParticles += particleRatio[0] * particlesPerCell;

  //upper half-blue particles to be added
  if(cellX > cellY)
    numParticles += particleRatio[1] * particlesPerCell;

  //along the diagonal
  if(cellX == cellY)
    numParticles += particleRatio[2] * particlesPerCell;

  int redBoxMin = (numCellsPerDim-(numCellsPerDim/4))/2;

  //condition for adding red particles
  if(cellX >= redBoxMin && cellX < redBoxMin+(numCellsPerDim/4) && cellY >= redBoxMin && cellY < redBoxMin +(numCellsPerDim/4))
    numParticles += particleRatio[3] * particlesPerCell;

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
  data[0]= numParticles;
  data[1]= numOutbound;
  data[2]= iteration;
  CkCallback cbTotalAndOutbound(CkIndex_Main::receiveTotalOutboundReductionData(NULL),mainProxy);

  contribute(3*sizeof(int), data, totalOutboundType, cbTotalAndOutbound);
}

void Cell::computeTotalParticles() {
  totalParticles = 0;
  for(int j=0; j < numCellsPerDim; j++) { // iterate over columns
    for(int i=0; i < numCellsPerDim; i++) { // iterate over rows
      totalParticles += computeParticlesInCell(i, j);
    }
  }
  DEBUG(CmiPrintf("[%d][%d] Total Particles are %d\n", thisIndex.x, thisIndex.y, totalParticles);)
}

void Cell::recvParticlesPostSimulation(vector<Particle> inbound) {

  if(reorgParticles.size() == 0) {
    reorgParticles.reserve(myShare);
  }

  reorgParticles.insert(reorgParticles.end(), inbound.begin(), inbound.end());

  if(reorgParticles.size() == myShare) {

    if(logOutput) {
      sortAndDump(outputFolderName);
    }

    verifyCorrectness();

  } else if(reorgParticles.size() > myShare) {
    CkAbort("[%d][%d] I currently have %lu particles, which is more than my share of %d particles\n", thisIndex.x, thisIndex.y, reorgParticles.size(), myShare);
  }
}

void Cell::readComparisonOutputFromFiles() {
  // Read precomputed particles for comparison
  string comparisonFile = "scripts/compareOutput";

  if(numCellsPerDim == 4) {
    comparisonFile += "/simple/";
  } else if(numCellsPerDim == 35) {
    comparisonFile += "/bench/";
  } else {
    CkPrintf("No comparison data available currently!\n");
  }

  comparisonFile += "sim_output_" + to_string(thisIndex.x) + "_" + to_string(thisIndex.y);

  DEBUG(CkPrintf("[%d][%d] Comparison file is %s\n", thisIndex.x, thisIndex.y, comparisonFile.c_str());)

  ifstream infile(comparisonFile);
  string line, token;
  Particle p;

  regex particleLine("(Particle)(.*)");
  precomputeParticles.reserve(myShare);

  while(getline(infile, line)) {

    if(regex_match(line, particleLine)) {

      istringstream iss1(line);
      getline(iss1, token, ':'); // discard 'Particle' text
      getline(iss1, token, ':');

      istringstream iss2(token);

      getline(iss2, token, ',');
      p.gid = stoi(token);

      getline(iss2, token, ',');
      p.x = stod(token);

      getline(iss2, token, ',');
      p.y = stod(token);

      getline(iss2, token, ',');
      p.color = token[0];

      precomputeParticles.push_back(p);
    }
  }
}

void Cell::verifyCorrectness() {

  // locally sort received reorg particles before comparison
  sort(reorgParticles.begin(), reorgParticles.end());

  readComparisonOutputFromFiles();

  // Verify correctness
  // Assert that number of particles is the same
  assert(precomputeParticles.size() == reorgParticles.size());

  for(int i=0; i < precomputeParticles.size(); i++) {
    assert(precomputeParticles[i].gid == reorgParticles[i].gid);
    assert(fabs(precomputeParticles[i].x - reorgParticles[i].x) < 1e-12);
    assert(fabs(precomputeParticles[i].y - reorgParticles[i].y) < 1e-12);

    if(fabs(precomputeParticles[i].y - reorgParticles[i].y) > 1e-12) {
      CkPrintf("[%d][%d] y coordinate difference greater than 1e-12 precompute y is %.*lf and reorg y is %.*lf gid is (%d, %d)\n", thisIndex.x, thisIndex.y, 14, precomputeParticles[i].y, 14, reorgParticles[i].y, precomputeParticles[i].gid, reorgParticles[i].gid);
    }
    assert(precomputeParticles[i].color == reorgParticles[i].color);
  }

  DEBUG(CkPrintf("[%d][%d] Correctness verified\n", thisIndex.x, thisIndex.y);)
  CkCallback doneCb(CkIndex_Main::done(), mainProxy);
  contribute(doneCb);
}

void Cell::sendParticlesPostSimulation(int linearCellId, vector<Particle> &outbound) {
  assert(linearCellId >= 0 && linearCellId < (numCellsPerDim * numCellsPerDim));

  int xCellId = linearCellId / numCellsPerDim;
  int yCellId = linearCellId % numCellsPerDim;
  thisProxy(xCellId, yCellId).recvParticlesPostSimulation(outbound);
}

void Cell::reorganizeParticles(int totalParticles, string subFolderName) {
  sort(particles.begin(), particles.end());
  outputFolderName = subFolderName;

  DEBUG(CkPrintf("[%d][%d] My share is %d\n", thisIndex.x, thisIndex.y, myShare);)

  int linearCellId = -1, prevLinearCellId = -1;
  vector<Particle> outbound;

  for(int i=0 ; i<particles.size(); i++) {

    linearCellId = (particles[i].gid - 1)/ppcEqualDist;

    if(linearCellId == numCellsPerDim * numCellsPerDim)
      linearCellId = linearCellId - 1;

    if(prevLinearCellId != linearCellId && prevLinearCellId != -1) {
      // Send the outbound particles
      sendParticlesPostSimulation(prevLinearCellId, outbound);
      outbound.clear();
    }

    outbound.push_back(particles[i]);
    prevLinearCellId = linearCellId;
  }

  // Send the last set of outbound particles
  if(outbound.size() != 0)
    sendParticlesPostSimulation(prevLinearCellId, outbound);
}

void Cell::sortAndDump(string subFolderName) {

  // sort particles before writing into files
  sort(reorgParticles.begin(), reorgParticles.end());

  // Create a file
  ofstream myFile;

  string myFileName = subFolderName + "/sim_output_" + to_string(thisIndex.x) + "_" + to_string(thisIndex.y);
  myFile.open(myFileName);

  if(myFile.is_open()) {
    myFile << "====================================== BEGIN ==========================================" << endl;
    myFile << "Cell:"<<thisIndex.x <<","<< thisIndex.y<< endl;
    myFile << "=======================================================================================" << endl;

    for(int i=0; i<reorgParticles.size(); i++) {
      DEBUG(CmiPrintf("[%d][%d] Final particle Sorted gid=%d => x=%lf, y=%lf, color=%c\n", thisIndex.x, thisIndex.y, reorgParticles[i].gid, reorgParticles[i].x, reorgParticles[i].y, reorgParticles[i].color);)
      myFile << "Particle:"<< reorgParticles[i].gid << fixed << setprecision(15) << ","<< reorgParticles[i].x << "," << reorgParticles[i].y << "," << reorgParticles[i].color << endl;
    }
    myFile << "====================================== END ==========================================" << endl;
  } else {
    CmiAbort("Error while opening the file for writing cell output");
  }
  myFile.close();

  //CkCallback doneCb(CkIndex_Main::done(), mainProxy);
  //contribute(doneCb);
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



