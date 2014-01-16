/*
 * FeatureGroup.h
 *
 *  Created on: Sep 23, 2013
 *      Author: stephan
 */

#ifndef FEATUREGROUP_H_
#define FEATUREGROUP_H_

#include "pnapi/pnapi.h"
#include "types.h"

#include <vector>

using namespace pnapi;
using std::vector;

class FeatureModel;

class FeatureGroup {
public:
	FeatureGroup(const GroupType &type, Feature *parent = NULL, FeatureModel *fm = NULL);
	virtual ~FeatureGroup();

	const GroupType &getType() const;

	void addFeature(Feature &feature);
	const vector<Feature *> &getFeatures() const { return features_; }

	void setParent(Feature &parent);
	Feature &getParent() const { return *parent_; }

private:
	void setType(const GroupType &type);

private:
	FeatureModel *fm_;
	// The Feature Group's type
	GroupType type_;
	// The Features belonging to the group
	vector<Feature *> features_;
	// The parent feature
	Feature *parent_;

	// Group Parent
	Place *parentAddFeature_;
};

#endif /* FEATUREGROUP_H_ */
