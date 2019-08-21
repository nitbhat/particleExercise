void Cell::populateCell(int initialElements) {
  //create random particles and add then to the particles vector

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
    double randomXPosition= startX + drand48()*(boxMax/numCellsPerDim);
    double randomYPosition= startY + drand48()*(boxMax/numCellsPerDim);
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
  //drand48 creates a random number between [0-1]
  double deltax = (drand48()-drand48())*10;
  double deltay = (drand48()-drand48())*10;

  if(particle->color=='r'){
    particle->x += deltax;
    particle->y += deltay;
  }
  else if(particle->color=='b'){
    particle->x += deltax/2;
    particle->y += deltax/2;
  }
  else if(particle->color=='g'){
    particle->x += deltax/4;
    particle->x += deltax/4;
  }
}
