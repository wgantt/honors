#include "../lcs.h"

/****************************************************************************
 * File:        lcsTest.cpp
 * Author:      Will Gantt
 * Description: Implements and runs various unit tests for functions of the
 * 		LCS class.
 ****************************************************************************/ 

using namespace std;

bool testDoGA(LCS lcs);                      // implemented
bool testDoCover(LCS lcs);                   // implemented
// bool testDoSpecify(LCS lcs);
bool testCover(LCS lcs);
void testRouletteWheelSelect(LCS lcs);       // implemented
bool testGaSubsume(LCS lcs);
bool testCreateMatchAndCorrectSets(LCS lcs);
bool testApplyGA(LCS lcs);
bool testProcessInput(LCS lcs);

// INITIALIZE GLOBALS HERE

// named constants (these should ultimately 
// be passed as parameters)
static const int MAX_POP_SIZE = 50;
static const int NUM_ATTRIBUTES = 10;
static const int NUM_DATA_POINTS = 10;

// random number generator and seed generator
mt19937 rng;
random_device rd;

// uniform distributions
uniform_real_distribution<double> real_dist(0,1);
uniform_int_distribution<int> int_dist(0,1);
uniform_int_distribution<int> int_dist2(1,25);

int main(int argc, char **argv) {

	rng.seed(rd());

	// create a random population and data set and
	// generate the LCS
	Population p = Population::random(MAX_POP_SIZE, NUM_ATTRIBUTES);
	Dataset d = Dataset::random(NUM_DATA_POINTS);
	LCS lcs = LCS(p,d);
	
	// run the tests
	testCover(lcs);

	return 0;
}

/****************************************************************************
 * Inputs:      The LCS on which the tests will be run.
 * Outputs:     A boolean indicating whether all tests have passed.
 * Description: Tests the DoGA function. This function indicates whether the
 * 		GA should be invoked on a particular iteration.
 ****************************************************************************/
bool testDoGA(LCS lcs) {

	int num_tests = 1;

	// set the current data point (this assumes a training set has
	// already been initialized) and print it.
	lcs.setCurrDataPoint(lcs.training_set_.data_points_[0]);
	Dataset::printDataPoint(lcs.curr_data_point(), NUM_ATTRIBUTES);

	// create the match set [M] and correct set [C]
	lcs.createMatchAndCorrectSets();

	// the number of rules in the correct set
	int correct_set_size = lcs.correct_set_.members_.size();

	// here, we artifically set the current generation to 100, which
	// should guarantee a true result from doGA if there are more than
	// two rules in [C]
	lcs.curr_gen_ = 100;

	// run doGA
	bool result_1 = lcs.doGA();

	// ----------------------------------------------------------------
	// TEST 1
	// ----------------------------------------------------------------
	printf("DoGA Test 1 of %d: ", num_tests);

	// if there are fewer than two rules in [C], the GA should obviously
	// not execute, as there are not enough rules to generate offspring
	if ((correct_set_size < 2) && (result_1 == false)) {
		printf("Passed.\n");
		return true;
	} else if ((correct_set_size < 2) && (result_1 == true)) {
		printf("Failed.\n");
		return false;
	
	// if we get here, we know that there are at least two rules in [C]
	// and we have to check additional conditions---to wit, how long it
	// has been on average since each rule in [C] was part of another
	// correct set on which the GA was called. Since we set the current
	// generation to 100, and since each rule is initialized with a time
	// stamp of 0, this should return true.
	} else if (result_1 == false) {
		printf("Failed.\n");
		return false;
	} else {
		printf("Passed.\n");
	}

	// NOTE: The code below will execute only if there are more than 
	// two rules in [C].
	
	// -----------------------------------------------------------------
	// TEST 2
	// -----------------------------------------------------------------
	
	// now set the current generation back to 0 and run the test a 
	// second time
	lcs.curr_gen_ = 0;

	// run doGA
	bool result_2 = lcs.doGA();

	printf("DoGA Test 2: ");
	if (result_2 == true) {
		printf("Failed.\n");
		return false;
	}

	printf("Passed.\n");
	return true;

} // end testDoGA

/****************************************************************************
 * Inputs:      The LCS on which the test is to be run.
 * Outputs:     A boolean indicating whether the test was passed.
 * Description: Tests the doCover function, which indicates whether the
 * 		covering operator should be invoked on a given iteration of
 * 		the GA.
 ****************************************************************************/
bool testDoCover(LCS lcs) {

	// the number of tests that will be run
	int num_tests = 1;

	// set the current data point (this assumes a training set has
	// already been initialized) and print it.
	lcs.setCurrDataPoint(lcs.training_set_.data_points_[0]);

	// create the match set [M] and correct set [C]
	lcs.createMatchAndCorrectSets();

	// the number of rules in the correct set
	int correct_set_size = lcs.correct_set_.members_.size();

	// run the test
	bool result = lcs.doCover();

	// evaluate results
	printf("DoCover Test 1 of %d: ", num_tests);
	if (((correct_set_size == 0) || 
			(lcs.match_set_.num_classes_represented() < lcs.theta_mna())) && 
			(result == false)) {
		printf("Failed. (Test returned false and should not have.)\n");
		return false;
	} else if (((correct_set_size > 0) && 
			(lcs.match_set_.num_classes_represented() >= lcs.theta_mna())) && 
			(result == true)) {
		printf("Failed. (Test returned true and should not have.)\n");
		return false;
	}


	printf("Passed.\n");
	return true;

} // end testDoCover

/****************************************************************************
 * Inputs:      The LCS on which the test is to be run.
 * Outputs:     A boolean indicating whether the test was passed.
 * Description: Tests the cover function, which generates a rule to match
 * 		the current input if and only if both of the following are
 * 		true:
 * 			1. [C] is empty.
 * 			2. The number of actions represented in the match
 * 			   set is less than the threshold value theta_mna_.
 *
 * 		It should be noted that this test relies on having working
 * 		createMatchAndCorrectSets and doCover functions. Thus, it
 * 		should be verified that these functions work before running
 * 		this test.
 ****************************************************************************/
bool testCover(LCS lcs) {

	// make sure there's enough room to add another rule to the population
	lcs.pop_.setMaxSize(lcs.pop_.max_size() + 1);

	// the number of tests that will be run
	int num_tests = 1;

	// a counter for the data point vector
	int i = 0;

	// the output of the doCover function
	bool doCover_result = false;

	// Keep iterating through the data set and constructing [M] and [C]
	// for the current data point, until we find a data point that
	// necessitates invoking the cover operator. Note that this loop
	// assumes that there will be *some* such data point.
	do {
		// set the current data point (this assumes a training set has
		// already been initialized) and print it.
		lcs.setCurrDataPoint(lcs.training_set_.data_points_[i]);

		// create the match set [M] and correct set [C]
		lcs.createMatchAndCorrectSets();

		// check to see whether the covering operator should be invoked
		doCover_result = lcs.doCover();

		// increment the data point counter
		i++;

	} while (!doCover_result);

	exit(0);

	// print the data point
	Dataset::printDataPoint(lcs.curr_data_point(), NUM_ATTRIBUTES);

	// the initial sizes of [P], [M], and [C]
	int orig_pop_size = lcs.pop_.size();
	int orig_match_set_size = lcs.match_set_.size();
	int orig_correct_set_size = lcs.correct_set_.size();

	// run the cover operator
	lcs.cover();

	// the new sizes of [P], [M], and [C] (if things are working correctly,
	// these should be just one greater than the initial sizes)
	int new_pop_size = lcs.pop_.size();
	int new_match_set_size = lcs.match_set_.size();
	int new_correct_set_size = lcs.correct_set_.size();
	
	// get the number of rules added to [P], [M], and [C]. This should
	// never be anything but 1.
	int pop_num_added = new_pop_size - orig_pop_size;
	int match_set_num_added = new_match_set_size - orig_match_set_size;
	int correct_set_num_added = new_correct_set_size - orig_correct_set_size;

	// ensure that the newly generated rule was added to [P], [M], and [C]
	printf("Cover Test 1 of %d: ", num_tests);
	if (pop_num_added != 1) {
		printf("Failed. (Rule was not added to population.)\n");
		return false;
	} else if (match_set_num_added != 1) {
		printf("Failed. (Rule was not added to [M].)\n");
		return false;
	} else if (correct_set_num_added != 1) {
		printf("Failed. (Rule was not added to [C].)\n");
		return false;
	}

	printf("Passed.\n");

	// print the newly generated rule
	lcs.correct_set_.print();

	return true;

} // end testCover

/****************************************************************************
 * Inputs:      the LCS on which the roulette wheel selection operator
 * Outputs:     a boolean indicating whether all of the tests of the
 * 		roulette wheel selection on the given LCS were passed
 * Description: runs a test of the rouletteWheelSelect function, which, as
 * 		its name suggests, randomly chooses a rule from the
 * 		population using fitness-based roulette wheel selection.
 * 		Note that this test doesn't return a boolean like most of
 * 		the others. That's because there's no hard criterion that
 * 		needs to be checked for truth or falsity; rather, we just
 * 		want to make sure that the function is generally selecting
 * 		the rules it should---i.e. those with high fitness relative
 * 		to the rest of the correct set.
 ****************************************************************************/ 
void testRouletteWheelSelect(LCS lcs) {
	
	// rouletteWheelSelect will segfault if it tries to run on an empty
	// correct set. This do-while loop iterates over inputs from the
	// training set until it finds one that generates a correct set with
	// at least one member.
	int correct_set_size = 0;
	int i=0;
	do {
		// set the current data point (this assumes a training set has
		// already been initialized) and print it.
		lcs.setCurrDataPoint(lcs.training_set_.data_points_[i]);
		Dataset::printDataPoint(lcs.curr_data_point(), NUM_ATTRIBUTES);

		// create the match set [M] and correct set [C]
		lcs.createMatchAndCorrectSets();
		correct_set_size = lcs.correct_set_.members_.size();
		i++;

	} while (correct_set_size == 0);

	// select three rules randomly from the population
	int r1_index = lcs.rouletteWheelSelect();
	int r2_index = lcs.rouletteWheelSelect();
	int r3_index = lcs.rouletteWheelSelect();

	// get their fitnesses
	double r1_fitness = lcs.pop_.rules_[r1_index].fitness();
	double r2_fitness = lcs.pop_.rules_[r2_index].fitness();
	double r3_fitness = lcs.pop_.rules_[r3_index].fitness();

	// the fitness rank of each of the three rules
	int r1_fitness_rank = correct_set_size;
	int r2_fitness_rank = correct_set_size;
	int r3_fitness_rank = correct_set_size;

	// a rule iterator variable and the rule's fitness
	Rule curr_rule;
	double curr_avg_niche_size;

	// iterate over the correct set
	int curr_rule_index;
	for (int i=0; i<correct_set_size; i++) {
		curr_rule_index = lcs.correct_set_.members_[i];
		curr_rule = lcs.pop_.rules_[curr_rule_index];
		curr_avg_niche_size = curr_rule.fitness();

	// the fitness rank of each of the three rules above is decremented
	// every time we encounter a rule in the population whose fitness is
	// less than that of the selected rule (in other words, the lower the
	// value of r<n>_fitness_rank, the higher the rank)
	if (curr_avg_niche_size < r1_fitness)
		r1_fitness_rank--;
	if (curr_avg_niche_size < r2_fitness)
		r2_fitness_rank--;
	if (curr_avg_niche_size < r3_fitness)
		r3_fitness_rank--;
	}

	// Tell user which rules were selected and their fitness ranks. The user
	// should not often expect to see rules with fitness ranks close to
	// correct_set_size, and should expect to frequently see rules with 
	// fitness ranks closer to 0.
	printf("Selected rule %d with fitness rank %d of %d\n",
			r1_index, r1_fitness_rank, correct_set_size);
	printf("Selected rule %d with fitness rank %d of %d\n",
			r2_index, r2_fitness_rank, correct_set_size);
	printf("Selected rule %d with fitness rank %d of %d\n",
			r3_index, r3_fitness_rank, correct_set_size);

} // end testRouletteWheelSelect

