/*
 * StageModel.h
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#ifndef STAGEMODEL_H_
#define STAGEMODEL_H_

#include <set>
#include <string>
#include <vector>
#include "pnapi/pnapi.h"
#include "tools.h"
#include "types.h"

using std::set;
using std::string;
using std::vector;
using namespace pnapi;

class Feature;
class Stage;

class StageModel {
public:
	StageModel(const vector<Feature *> &config, const Strategy &strat, const InnerStrategy &istrat, Feature *root);
	virtual ~StageModel();

	void addFeature(Feature *f);

	void setup();
	PetriNet &buildNet();
	void computeRecursive();

	void shift(const int pos);
	void shiftBack(const int pos);
	void shiftLeft(const int pos);
	void shiftRight(const int pos);
	void shiftRight2(const int pos);

	bool isSound() const { return isSound_; }
	const PetriNet &getPetriNet() const { return *stageNet_; }

	const string toString() const;

	const int size() const { return stages_.size(); }
	const int visited() const { return hashMap_.size(); }
	const float ratio() const;
	const int min() const;
	const int max() const;

	const int numFeatures() const;

	const int tried() const { return triedOnes_; }
	const string getProductSequence() const;

private:
	// Set of Stages
	vector<Stage *> stages_;
	// Creation Strategy
	Strategy strat_;
	// Backtracking Strategy
	InnerStrategy istrat_;

	// Root Feature
	Feature *root_;
	// Configuration
	vector<Feature *> configuration_;

	PetriNet *stageNet_;
	// Soundness Flag
	bool isSound_;
	// hash map
	set<string> hashMap_;

	// number of tried (sequences)
	int triedOnes_;
	int soundOnes_;

};

#endif /* STAGEMODEL_H_ */
