#ifndef PARTICLE_H
#define PARTICLE_H

/*
*Particle object with x&y coordinate components
*/

class Particle  {
public:
    int gid;  // unique global particle id
    double x; // x coordinate
    double y; // y coordinate
    char color; // color

    Particle() { }
    Particle(double a, double b, char color, int id) {
      gid = id;
      x=a; y=b;
      this->color=color;
    }

    void pup(PUP::er &p){
      p|gid;
      p|x;
      p|y;
      p|color;
    }
};

#endif
