/*
 * RavenTools.C
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#include "RavenTools.h"

using namespace std;

template<>
InputParameters validParams<RavenTools>(){

  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("type","Raven Tool type");
  params.registerBase("RavenTools");
  return params;
}

RavenTools::RavenTools(const std::string & name, InputParameters parameters):
    MooseObject(name,parameters)
{
  _type=getParam<std::string>("type");
}

RavenTools::~RavenTools()
{
}
double
RavenTools::getVariable(std::string variableName)
{
  double res;
  if(_tool_parameters.find(variableName) != _tool_parameters.end()){
     res = _tool_parameters.find(variableName) ->second;
  }
  else{
    mooseError("Parameter " << variableName << " was not found in RavenTool type " << _type <<".");
  }
  return res;
}

std::vector<std::string>
RavenTools::getVariableNames(){
  std::vector<std::string> paramtersNames;
  for (std::map<std::string,double>::iterator it = _tool_parameters.begin(); it!= _tool_parameters.end();it++){
    paramtersNames.push_back(it->first);
  }
  return paramtersNames;
}


void
RavenTools::updateVariable(std::string variableName, double & newValue)
{
  if(_tool_parameters.find(variableName) != _tool_parameters.end()){
    // we are sure the variableName is already present in the mapping =>
    // we can update it in the following way
    _tool_parameters[variableName] = newValue;
  }
  else{
    mooseError("Parameter " << variableName << " was not found in RavenTool type " << _type << ".");
  }
}

double
RavenTools::compute(double value){return value;}

std::string &
RavenTools::getType()
{
  return _type;
}
/*
 * external functions for Python interface
 */
std::string
getRavenToolType(RavenTools & tool)
{
  return tool.getType();
}
double
getRavenToolVariable(RavenTools & tool,const std::string & variableName)
{
  return tool.getVariable(variableName);
}

void
updateRavenToolVariable(RavenTools & tool,const std::string & variableName, double newValue)
{
  tool.updateVariable(variableName,newValue);
}

double
computeRavenTool(RavenTools & tool,double value)
{
  return tool.compute(value);
}
std::vector<std::string>
getToolVariableNames(RavenTools & tool)
{
  return tool.getVariableNames();
}

