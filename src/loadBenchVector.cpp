#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

//#include "pup.h"
#include "particle.h"

using namespace std;

int main() {
  cout<<"Hello World!"<<endl;
  ifstream infile("/scratch/users/nbhat4/particleSimulation/scripts/compareOutput/bench/sim_output_combined_sorted");


  vector<Particle> particles;
  particles.reserve(34750000);
  string line, text;

  Particle p;
  int i=0;

  while(getline(infile, line)) {
    istringstream iss(line);

    iss >> text;

    iss >> p.gid;
    iss >> p.x;
    iss >> p.y;
    iss >> p.color;

    particles.push_back(p);
  }

  for(int i=0; i< particles.size(); i++) {
      //cout<< "Particle "<< particles[i].gid << "," << particles[i].color << "," << particles[i].x << "," << particles[i].y << endl;
  }

  return 0;
}
