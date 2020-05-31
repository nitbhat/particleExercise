#include "main.h"
#include "cell.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>

/*readonly*/ CProxy_Main mainProxy;
/*readonly*/ CProxy_Cell cellProxy;
/*readonly*/ int particlesPerCell;
/*readonly*/ int numCellsPerDim;
/*readonly*/ int iterations;
/*readonly*/ int lbFreq;
/*readonly*/ int reductionFreq;
/*readonly*/ double boxMax;
/*readonly*/ double boxMin;
/*readonly*/ double cellDim;
/*readonly*/ int velocityFactor;
/*readonly*/ vector<int> particleRatio;
/*readonly*/ bool logOutput;

#if LIVEVIZ_RUN
/*readonly*/ double pixelScale;
#endif

CkReduction::reducerType totalOutboundType;

CkReduction::reducerType minMaxType;

Main::Main(CkArgMsg* m) {
  if(m->argc < 8) CkAbort("USAGE: ./charmrun +p<number_of_processors> ./particle <number of particles per cell> <size of array> <numIterations> <lower, upper, diag, box> <vel-factor> <output-prompt> <load balancing Frequency>");

  mainProxy = thisProxy;
  particlesPerCell = atoi(m->argv[1]);
  numCellsPerDim = atoi(m->argv[2]);
  iterations = atoi(m->argv[3]);
  string particleRatioStr(m->argv[4]);
  velocityFactor = atoi(m->argv[5]);
  string logOutputString(m->argv[6]);
  lbFreq = atoi(m->argv[7]);
  delete m;

  stringstream ss(particleRatioStr);

  for (int i; ss >> i;) {
    assert(i >= 0);
    particleRatio.push_back(i);
    if (ss.peek() == ',')
      ss.ignore();
  }

  if(particleRatio.size() != 4)
    CkAbort("Particle ratio input incorrect! Pass particle ratio input as a comma seprated string <upper, lower, diag, box>");

  if (logOutputString == "yes") {
    logOutput = true;
  } else if(logOutputString == "no") {
    logOutput = false;
  } else {
    CkAbort("log-output incorrect! Pass either \"yes\" or \"no\"");
  }

  // Each cell has 1.0 * 1.0 dimensions and hence the total box dimensions will be 0.0 and 1.0 * numCellsPerDim
  // declare box dimensions
  boxMax = numCellsPerDim * 1.0;
  boxMin = 0.0;

  cellDim = 1.0;

  minParticles = -1;
  maxParticles = -1;

  minCellX = -1;
  minCellY = -1;

  maxCellX = -1;
  maxCellY = -1;

  reductionFreq = 5;

  totalParticles = -1;

  CkPrintf("================================ Input Params ===============================\n");
  CkPrintf("====================== Particles In A Box Simulation ========================\n");
  CkPrintf("Grid Size                                                  = %d X %d\n", numCellsPerDim, numCellsPerDim);
  CkPrintf("Particles/Cell seed value                                  = %d\n", particlesPerCell);
  CkPrintf("Number of Iterations                                       = %d\n", iterations);
  CkPrintf("Green Particles (Lower Triangular Half) distribution ratio = %d\n", particleRatio[0]);
  CkPrintf("Blue Particles  (Upper Triangular Half) distribution ratio = %d\n", particleRatio[1]);
  CkPrintf("Red Particles   (Diagonal) distribution ratio              = %d\n", particleRatio[2]);
  CkPrintf("Red Particles   (Central Box) distribution ratio           = %d\n", particleRatio[3]);
  CkPrintf("Velocity Reduction Factor                                  = %d\n", velocityFactor);
  CkPrintf("Log Output                                                 = %d\n", logOutput);
  CkPrintf("Load Balancing Frequency                                   = %d\n", lbFreq);
  CkPrintf("=============================================================================\n");
  CkPrintf("======================= Launching Particle Simulation =======================\n");


  //declare a 2D chare array with dimensions numCellsPerDim*numCellsPerDim
  CkArrayOptions opts(numCellsPerDim, numCellsPerDim);
  cellProxy = CProxy_Cell::ckNew(opts);

#if LIVEVIZ_RUN
  pixelScale  = 100.0;
  CkCallback c(CkIndex_Cell::mapChareToImage(0), cellProxy);
  liveVizConfig cfg(liveVizConfig::pix_color, true);
  liveVizInit(cfg, cellProxy, c, opts);
#endif

  startTime = CkWallTimer();

  //start the run for all the chares
  cellProxy.run();
}

//function to receive the reduction result
void Main::receiveTotalOutboundReductionData(CkReductionMsg *data){
  int *output = (int *) data->getData();
  //CkAssert(output[2] == particlesPerCell*numCellsPerDim*numCellsPerDim);
  printTotal(output[0], output[1], output[2]);
  if(output[2] == iterations) {
    endTime = CkWallTimer();

    totalParticles = output[0];

    totalTime = (endTime - startTime);
    CkPrintf("======================= Particle Simulation Complete ========================\n");
    CkPrintf("Simulation Complete, total time taken is %lf seconds\n", totalTime);
    CkPrintf("=============================================================================\n");
#if BONUS_QUESTION
    // Broadcast everyone to contribute to bonus question reduction
    cellProxy.contributeToReduction();
#else
    readyToOutput();
#endif
  }
}

bool Main::getUserInput() {
  char writeToFile[20];
  CmiPrintf("Do you want to log the final output data? (yes/no) :");
  CkScanf("%s", writeToFile);
  string userInputPrompt(writeToFile);

  if (userInputPrompt == "yes") {
    return true;
  } else if(userInputPrompt == "no") {
    return false;
  } else {
    CmiPrintf("User Input incorrect! Try again, Pass either \"yes\" or \"no\"\n");
    return getUserInput();
  }
}

string Main::getDefaultSubdirectoryName() {
  char runOutputFolder[80];
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t curtime = tv.tv_sec;
  struct tm *now = localtime(&curtime);
  string folderName = "sim_output_%H-%M-%S-" + to_string(tv.tv_usec) + "-" + to_string(numCellsPerDim) +"-" + to_string(particlesPerCell) +"-" + to_string(iterations);

  strftime(runOutputFolder, 80, folderName.c_str(), now);
  string name(runOutputFolder);
  return name;
}

void Main::readyToOutput() {
  struct stat info;
  if(stat("output", &info) != 0) {
    // Create an output directory
    const int mkdirOut = mkdir("output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (-1 == mkdirOut) {
      CmiAbort("Error while creating the output directory");
    }
  }

  char userRunOutputFolder[80];
  string folderName;

  folderName = getDefaultSubdirectoryName();

  string parentFolder("output/");
  finalPath = parentFolder + folderName;

  // Create an output subdirectory that is dependent on the current time
  const int mkdirOut = mkdir(finalPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (-1 == mkdirOut) {
    CmiAbort("Error while creating the output sub-directory");
  }

  // Write the performance data in a file
  string myFileName = finalPath + "/sim_output_main";

  // Create a file
  ofstream myFile;
  myFile.open(myFileName);

  if(myFile.is_open()) {
    myFile << "====================================== BEGIN ==========================================" << endl;
    myFile << "Main:" << endl;
    myFile << "=======================================================================================" << endl;
    myFile << "Input:Grid Size:" << numCellsPerDim << endl;
    myFile << "Input:Particles Per Cell Seed:" << particlesPerCell << endl;
    myFile << "Input:Number Of Iterations:" << iterations << endl;
    myFile << "Input:Particle Ratio:" << particleRatio[0] << "," << particleRatio[1] << ",";
    myFile << particleRatio[2] << "," << particleRatio[3] << endl;
    myFile << "Input:Velocity Factor:" << velocityFactor << endl;
    myFile << "Output:Total Time:" << totalTime << endl;
    myFile << "Output:Time Per Step:" << totalTime/iterations << endl;
    myFile << "Output:Max Particles:" << maxParticles << endl;
    myFile << "Output:Cell with Max Particles:" << "(" << maxCellX << "," << maxCellY << ")" << endl;
    myFile << "Output:Min Particles:" << minParticles << endl;
    myFile << "Output:Cell with Min Particles:" << "(" << minCellX << "," << minCellY << ")" << endl;
    myFile << "====================================== END ==========================================" << endl;
  } else {
    CmiAbort("Error while opening the file for writing main output");
  }

  myFile.close();

  // Ask every cell to send the particles to the right home based on the global index
  cellProxy.reorganizeParticles(finalPath);
}

void Main::done() {
  CkPrintf("=============================================================================\n");
  CkPrintf("Success! Simulation correctness verified across all cells\n");
  CkPrintf("=============================================================================\n");
  CkPrintf("Final summarized output has been written to: %s/sim_output_main\n", finalPath.c_str());
  if(logOutput) {
    CkPrintf("All particle output has been written to files in directory : %s\n", finalPath.c_str());
  }
  CkPrintf("Exiting program\n");
  CkExit();
}

// and max counts and exiting when the iterations are done
void Main::printTotal(int total, int max, int iter){
  CkPrintf("Iteration: %d, Outgoing Particles Sum: %d, Total Particles: %d\n", iter, max, total);
}

// Global Functions
CkReductionMsg *calculateTotalAndOutbound(int nMsg, CkReductionMsg **msgs) {
  int returnVal[3];

  //signifies total particles sum value
  returnVal[0]=0;

  //signifies outgoing particles sum value
  returnVal[1]=0;

  for (int i=0;i<nMsg;i++) {
    CkAssert(msgs[i]->getSize()==3*sizeof(int));
    int *m=(int *)msgs[i]->getData();

    returnVal[0]+=m[0]; // Sum of total particles

    returnVal[1]+=m[1]; // Sum of outbound particles

    returnVal[2]=m[2];
  }
  return CkReductionMsg::buildNew(3*sizeof(int),returnVal);
}

CkReductionMsg *calculateMaxMin(int nMsg, CkReductionMsg **msgs);

void registerCalculateTotalAndOutbound(void){
  totalOutboundType = CkReduction::addReducer(calculateTotalAndOutbound);
#if BONUS_QUESTION
  minMaxType = CkReduction::addReducer(calculateMaxMin);
#endif
}

#include "particleSimulation.def.h"
