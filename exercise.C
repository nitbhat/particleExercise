//change the position of the particles and send messages to neighbors with their incoming particles
void Cell::updateParticles(int iter) {

  //declaring vectors for particles whose position needs to be changed
  std::vector<Particle> topLeft, top, topRight, left, right, bottomLeft, bottom, bottomRight;
  int newXIndex, newYIndex;

  // Iterate over the particles and call 'perturb'
  int swapIndex = particles.size()-1;
  for(int i=particles.size()-1;i>=0; i--){

    //change the position of the particle
    perturb(&particles[i]);

    //find the cell the particle belongs
    //condition for shifting to top-left cell
    if(particles[i].x<startX && particles[i].y<startY){
      topLeft.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to top cell
    else if(particles[i].x>startX && particles[i].x<startX + (boxMax/numCellsPerDim) && particles[i].y<startY){
      top.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to top-right cell
    else if(particles[i].x>startX+(boxMax/numCellsPerDim) && particles[i].y<startY){
      topRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to left cell
    else if(particles[i].x < startX && particles[i].y>startY && particles[i].y<startY+ (boxMax/numCellsPerDim)){
      left.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to right cell
    else if(particles[i].x>startX+(boxMax/numCellsPerDim) && particles[i].y>startY && particles[i].y<startY + (boxMax/numCellsPerDim))
    {
      right.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-left cell
    else if(particles[i].x<startX && particles[i].y>startY+(boxMax/numCellsPerDim)){
      bottomLeft.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom cell
    else if(particles[i].x>startX && particles[i].x<startX+(boxMax/numCellsPerDim) && particles[i].y>startY+ (boxMax/numCellsPerDim)){
      bottom.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-right cell
    else if(particles[i].x >startX+ (boxMax/numCellsPerDim) && particles[i].y>startY+ (boxMax/numCellsPerDim)){
      bottomRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }
  }
  particles.resize(swapIndex+1);

  //shifting the particles to the correct cells
  newXIndex = thisIndex.x-1;
  newYIndex = thisIndex.y-1;
  if(newXIndex==-1) newXIndex=numCellsPerDim-1;
  if(newYIndex==-1) newYIndex=numCellsPerDim-1;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,topLeft);

  newXIndex = thisIndex.x;
  newYIndex = thisIndex.y-1;
  if(newYIndex==-1) newYIndex=numCellsPerDim-1;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,top);

  newXIndex = thisIndex.x+1;
  newYIndex = thisIndex.y-1;
  if(newXIndex==numCellsPerDim) newXIndex=0;
  if(newYIndex==-1) newYIndex=numCellsPerDim-1;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,topRight);

  newXIndex = thisIndex.x-1;
  newYIndex = thisIndex.y;
  if(newXIndex==-1) newXIndex=numCellsPerDim-1;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,left);

  newXIndex = thisIndex.x+1;
  newYIndex = thisIndex.y;
  if(newXIndex==numCellsPerDim) newXIndex=0;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,right);

  newXIndex = thisIndex.x-1;
  newYIndex = thisIndex.y+1;
  if(newXIndex==-1) newXIndex=numCellsPerDim-1;
  if(newYIndex==numCellsPerDim) newYIndex=0;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottomLeft);

  newXIndex = thisIndex.x;
  newYIndex = thisIndex.y+1;
  if(newYIndex==numCellsPerDim) newYIndex=0;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottom);

  newXIndex = thisIndex.x+1;
  newYIndex = thisIndex.y+1;
  if(newXIndex==numCellsPerDim) newXIndex=0;
  if(newYIndex==numCellsPerDim) newYIndex=0;
  thisProxy(newXIndex, newYIndex).receiveUpdate(iter,bottomRight);
}


