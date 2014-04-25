/*
 * CrowTools_min.h
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#ifndef CROWTOOLS_MIN_H_
#define CROWTOOLS_MIN_H_

#include <string>

class CrowTools;

/*
 * external functions for Python interface
 */
std::string getCrowToolType(CrowTools & tool);
double      getCrowToolVariable(CrowTools & tool,const std::string & variableName);
void        updateCrowToolVariable(CrowTools & tool,const std::string & variableName, double newValue);
double      computeCrowTool(CrowTools & tool,double value);
std::vector<std::string> getToolVariableNames(CrowTools & tool);

#endif /* CROWTOOLS_MIN_H_ */
