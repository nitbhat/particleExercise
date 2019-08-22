int wrap(int index) {
  if(index == -1) return numCellsPerDim - 1;
  else if(index == numCellsPerDim) return 0;
  else return index;
}

void Cell::updateParticles(int iter) {

  //TODO: Add code for the following
  // 1. Iterate through the particles and call perturb(...) passing each particle. This causes the particle's x and y coordinate to change
  // 2. Identify the new cell that the particle belongs to and construct a vector of particles to be sent to each of the 8 neighbors (topLeft, top, topRight, left, right, bottomLeft, bottom, bottomRight)
  // 3. Make sure that particles that go outside the bounding box are wrapped back i.e for a 2D box consisting of 8 cells in each dimension,
  //    if a particle with the index (7,7) goes to (7, 8), it should be sent back to (7, 0).
  // 3. Call sendParticles(...) to send the 8 different vector of particles to each of the 8 neighbours

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
    else if(particles[i].x>startX && particles[i].x<startX + (cellDim) && particles[i].y<startY){
      top.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to top-right cell
    else if(particles[i].x>startX+(cellDim) && particles[i].y<startY){
      topRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to left cell
    else if(particles[i].x < startX && particles[i].y>startY && particles[i].y<startY+ (cellDim)){
      left.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to right cell
    else if(particles[i].x>startX+(cellDim) && particles[i].y>startY && particles[i].y<startY + (cellDim))
    {
      right.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-left cell
    else if(particles[i].x<startX && particles[i].y>startY+(cellDim)){
      bottomLeft.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom cell
    else if(particles[i].x>startX && particles[i].x<startX+(cellDim) && particles[i].y>startY+ (cellDim)){
      bottom.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }

    //condition for shifting to bottom-right cell
    else if(particles[i].x >startX+ (cellDim) && particles[i].y>startY+ (cellDim)){
      bottomRight.push_back(particles[i]);
      swap(particles[i], particles[swapIndex--]);
    }
  }
  particles.resize(swapIndex+1);

  int myX = thisIndex.x;
  int myY = thisIndex.y;

  // Send particles to my top row
  sendParticles(wrap(myX - 1), wrap(myY - 1), iter, topLeft);
  sendParticles(myX    , wrap(myY - 1), iter, top);
  sendParticles(wrap(myX + 1), wrap(myY - 1), iter, topRight);

  // Send particles to my row (left and right)
  sendParticles(wrap(myX - 1), myY    , iter, left);
  sendParticles(wrap(myX + 1), myY    , iter, right);

  // Send particles to my bottom row
  sendParticles(wrap(myX - 1), wrap(myY + 1), iter, bottomLeft);
  sendParticles(myX    , wrap(myY + 1), iter, bottom);
  sendParticles(wrap(myX - 1), wrap(myY + 1), iter, bottomRight);
}
