/*
 * CrowTools.C
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#include "CrowTools.h"

using namespace std;

template<>
InputParameters validParams<CrowTools>(){

  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("type","Crow Tool type");
  params.registerBase("CrowTools");
  return params;
}

CrowTools::CrowTools(const std::string & name, InputParameters parameters):
    MooseObject(name,parameters)
{
  _type=getParam<std::string>("type");
}

CrowTools::~CrowTools()
{
}
double
CrowTools::getVariable(std::string variableName)
{
  double res;
  if(_tool_parameters.find(variableName) != _tool_parameters.end()){
     res = _tool_parameters.find(variableName) ->second;
  }
  else{
    mooseError("Parameter " << variableName << " was not found in CrowTool type " << _type <<".");
  }
  return res;
}

std::vector<std::string>
CrowTools::getVariableNames(){
  std::vector<std::string> paramtersNames;
  for (std::map<std::string,double>::iterator it = _tool_parameters.begin(); it!= _tool_parameters.end();it++){
    paramtersNames.push_back(it->first);
  }
  return paramtersNames;
}


void
CrowTools::updateVariable(std::string variableName, double & newValue)
{
  if(_tool_parameters.find(variableName) != _tool_parameters.end()){
    // we are sure the variableName is already present in the mapping =>
    // we can update it in the following way
    _tool_parameters[variableName] = newValue;
  }
  else{
    mooseError("Parameter " << variableName << " was not found in CrowTool type " << _type << ".");
  }
}

double
CrowTools::compute(double value){return value;}

std::string &
CrowTools::getType()
{
  return _type;
}
/*
 * external functions for Python interface
 */
std::string
getCrowToolType(CrowTools & tool)
{
  return tool.getType();
}
double
getCrowToolVariable(CrowTools & tool,const std::string & variableName)
{
  return tool.getVariable(variableName);
}

void
updateCrowToolVariable(CrowTools & tool,const std::string & variableName, double newValue)
{
  tool.updateVariable(variableName,newValue);
}

double
computeCrowTool(CrowTools & tool,double value)
{
  return tool.compute(value);
}
std::vector<std::string>
getToolVariableNames(CrowTools & tool)
{
  return tool.getVariableNames();
}

