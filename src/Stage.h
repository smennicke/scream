/*
 * Stage.h
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#ifndef STAGE_H_
#define STAGE_H_

#include <set>
#include <string>
#include <vector>
#include "pnapi/pnapi.h"

using std::set;
using std::string;
using std::vector;
using namespace pnapi;

class Feature;

class Stage {
public:
	Stage();
	virtual ~Stage();

	PetriNet &getWorkflow(int stage, set<Feature *> added);

	void push_back(Feature *f);
	void push_front(Feature *f);

	Feature *pop_back();
	Feature *pop_front();

	const string getInput() const { return i_; }
	const string getOutput() const {return o_; }

	const int size() const { return features_.size(); }
	const string toString() const;

	const vector<Feature *> getFeatures() const { return features_; }

private:
	// A set of features (i.e. feature operations)
	vector<Feature *> features_;

	// Stage Net
	PetriNet *net_;
	// Stage Input
	string i_;
	// Stage Output
	string o_;

};

#endif /* STAGE_H_ */
