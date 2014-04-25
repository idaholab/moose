/*
 * RavenTools_min.h
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#ifndef RAVENTOOLS_MIN_H_
#define RAVENTOOLS_MIN_H_

#include <string>

class RavenTools;

/*
 * external functions for Python interface
 */
std::string getRavenToolType(RavenTools & tool);
double      getRavenToolVariable(RavenTools & tool,const std::string & variableName);
void        updateRavenToolVariable(RavenTools & tool,const std::string & variableName, double newValue);
double      computeRavenTool(RavenTools & tool,double value);
std::vector<std::string> getToolVariableNames(RavenTools & tool);

#endif /* RAVENTOOLS_MIN_H_ */
