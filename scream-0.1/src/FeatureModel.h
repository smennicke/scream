/*
 * FeatureModel.h
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#ifndef FEATUREMODEL_H_
#define FEATUREMODEL_H_

#include <string>
#include <vector>
#include "pnapi/pnapi.h"
#include "types.h"

using namespace pnapi;
using std::string;
using std::vector;

class Feature;
class FeatureGroup;
class StageModel;

class FeatureModel {
public:
	FeatureModel(Feature *root = NULL);
	virtual ~FeatureModel();

	void setRootFeature(Feature &root);
	Feature &getRootFeature() const;

	Feature &addFeature(const FeatureType &type = OPTIONAL, const string &name = "", Feature *parent = NULL);
	FeatureGroup &addGroup(const GroupType &type, Feature *parent = NULL);
	Feature &addGroupFeature(FeatureGroup &group, const string &name = "");

	PetriNet &getPetriNet();
	void buildPetriNet();

	bool checkConfiguration(const string &conf = "") const;
	StageModel &computeStageModel(vector<Feature *> config = vector<Feature *>(),
			const Strategy &strat = FORWARDS, const InnerStrategy &istrat = LAST);

	Feature *findFeature(const string &name);

	const int size() const { return features_.size(); }
	const float ctcr() const;
	void addConstraint() { ctcs_++; }

private:
	// the root feature
	Feature *root_;
	// set of all features
	vector<Feature *> features_;

	// the FM's PN representation
	PetriNet *net_;

	// counter for CTCs
	int ctcs_;

};

#endif /* FEATUREMODEL_H_ */
