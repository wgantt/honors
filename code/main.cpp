#include "lcs.h"
#include <time.h>

/****************************************************************************
 * File:        main.cpp
 * Author:      William Gantt
 * Description: 
 *
 * TODO:
 * 	- fix member variable names in all necessary files
 * 	- make member variables private that should be private
 * 	- add comments to the population and LCS files (esp function 
 * 	  descriptions)
 * 	- Update Crisp_LCS_Schema.txt
 * 	- remove using namespace from header files
 *
 ****************************************************************************/ 
  
using namespace std;
   
// declared as external variables in utilities.h, defined here
mt19937 rng;
random_device rd;
uniform_real_distribution<double> real_dist(0,1);
uniform_int_distribution<int> int_dist(1,10);

/****************************************************************************
 *
 *
 *
 ****************************************************************************/ 
int main(int argc, char **argv) {
   
	rng.seed(rd());

	LCS lcs(10);
	lcs.pop_ = Population::random(10,10);
	lcs.setCurrDataPoint(Dataset::randomDataPoint(10));
	lcs.reproduceAndReplace();

	return 0;
	
}
