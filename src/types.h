/*
 * types.h
 *
 *  Created on: Sep 22, 2013
 *      Author: stephan
 */

#ifndef TYPES_H_
#define TYPES_H_

enum FeatureType { MANDATORY, OPTIONAL, IN_GROUP };

enum GroupType { OR, XOR };

enum Strategy { FORWARDS, BACKWARDS };

enum InnerStrategy { FIRST, LAST, FORELAST };


#endif /* TYPES_H_ */
