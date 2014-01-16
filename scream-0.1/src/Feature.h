/*
 * Feature.h
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#ifndef FEATURE_H_
#define FEATURE_H_

#include "types.h"

#include <map>
#include <string>
#include <vector>
#include "pnapi/pnapi.h"

using std::map;
using std::string;
using std::vector;
using namespace pnapi;

class FeatureGroup;
class FeatureModel;

/*
 * Base Class: Feature
 */
class Feature {
public:
	Feature(const FeatureType &type = OPTIONAL, const string &name = "", Feature *parent = NULL, FeatureGroup *group = NULL);
	Feature(const string &name, FeatureGroup *group = NULL);
	virtual ~Feature();

	void setType(const FeatureType &type);
	const FeatureType getType() const { return type_; }

	void setName(const string &name);
	const string &getName() const;

	void setParent(Feature &parent);
	Feature &getParent() const;
	bool isRoot() const { return (parent_ == NULL); }

	void setGroup(FeatureGroup &group);
	FeatureGroup *getGroup() const { return group_; }

	const vector<Feature *> & getChildren() const;

	void requires(Feature &f);
	const vector<Feature *> &getRequires() const;

	const vector<Feature *> &getRequiredBy() const;

	void excludes(Feature &f);
	const vector<Feature *> &getExcludes() const;

	Transition &getAddFeature() const { return *addFeature_; }
	Transition &getRemoveFeature() const { return *removeFeature_; }
	Place &getInputFeature() const { return *inputFeature_; }
	Place &getOutputFeature() const { return *outputFeature_; }

	PetriNet &getPattern() const { return *netPattern_; }

private:
	void addChild(Feature &child);
	void requiredBy(Feature &f);
	void excludedBy(Feature &f);

private:
	// Feature's Type
	FeatureType type_;
	// Feature's Name
	string name_;
	// Group the feature is in
	FeatureGroup *group_;

	// Parent Feature
	Feature *parent_;
	// Child Features
	vector<Feature *> children_;

	// Requires set
	vector<Feature *> requires_;
	// Required by set
	vector<Feature *> requiredBy_;
	// excludes set
	vector<Feature *> excludes_;

	// Petri net related elements
	Place *inputFeature_;
	Place *outputFeature_;
	Transition *addFeature_;
	Transition *removeFeature_;

	PetriNet *netPattern_;

};

#endif /* FEATURE_H_ */
