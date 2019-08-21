#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  int i;          //loop counter
  double num;        //store random number

  srand48(1);

  //randomize();    //call it once to generate random number
  for(i=1;i<=10;i++)
  {
    num=drand48(); //get random number
    std::cout << num << "\t";
  }
  std::cout << "\n";
  return 0;
}
