#ifndef MATCHSET_H
#define MATCHSET_H

#include "rule.h"
#include "population.h"
#include "utilities.h"

/****************************************************************************
 * File:        matchSet.h
 * Author:      Will Gantt
 * Description: Defines the MatchSet class. The match set [M] is the set of
 * 		all rules in [P] whose condition matches that of the current
 * 		input. A rule is said to match an input when all of the
 * 		input's attribute values fall within the range of the
 * 		corresponding attribute in the rule's condition.
 ****************************************************************************/ 
class MatchSet {

	public:

		// default constructor
		MatchSet() { };

		// custom constructor
		MatchSet(Population *p) : p_(p), exp_sum_(0), avg_exp_(0), 
			num_classes_represented_(0) { };

		// the population from which the match set is created
		Population *p_;
		
		// stores the indices of the rules in the
		// general population that belong to the match set
		std::vector<int> members_;

		// indicates which class attribute values are
		// represented in the match set
		bool classes_represented_[NUM_CLASSES];

		// adds a rule's index to the match set and
		// updates the relevant member variables
		void add(int index);

		// resets the match set (but maintains the pointer to the population)
		void clear() {
			exp_sum_ = 0;
			avg_exp_ = 0;
			members_.clear();
			for (int i=0; i<NUM_CLASSES; i++)
				classes_represented_[i] = false;
		}

		// returns the size of the match set
		int size() {return members_.size();}

		// indicates whether the match set is empty
		bool isEmpty() {return members_.empty();}
			
		// prints the match set
		void print() {
			printf("\nMATCH SET\n---------\n");
			int num_members = members_.size();
			for (int i=0; i<num_members; i++)
				(p_->rules_[members_[i]]).printVerbose();
		}

		// getters
		double exp_sum() {return exp_sum_;}
		double avg_exp() {return avg_exp_;}
		int num_classes_represented() {return num_classes_represented_;}

		// setters
		void setExpSum(double exp_sum) {
			exp_sum_ = exp_sum;
		}
		void setAvgExp(double avg_exp) {
			avg_exp_ = avg_exp;
		}

	private:

		// the sum of the experience values of all the rules in [M]
		double exp_sum_;

		// the average of the experience values of all the rules in [M]
		double avg_exp_;

		// the number of distinct classes represented by the rules in [M]
		int num_classes_represented_;
};

#endif
