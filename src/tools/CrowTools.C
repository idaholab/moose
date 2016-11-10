/*
 * CrowTools.C
 *
 *  Created on: May 28, 2013
 *      Author: alfoa
 */

#include "CrowTools.h"
#include <iostream>

template<>
InputParameters validParams<CrowTools>(){

  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("type","Crow Tool type");
  params.registerBase("CrowTools");
  return params;
}

CrowTools::CrowTools(const InputParameters & parameters):
    MooseObject(parameters)
{
  _type=getParam<std::string>("type");
}

CrowTools::~CrowTools()
{
}
double
CrowTools::getVariable(std::string variable_name)
{
  double res;
  if(_tool_parameters.find(variable_name) != _tool_parameters.end()){
     res = _tool_parameters.find(variable_name) ->second;
  }
  else{
    mooseError("Parameter " << variable_name << " was not found in CrowTool type " << _type <<".");
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
CrowTools::updateVariable(const std::string variable_name, double new_value)
{
  if(_tool_parameters.find(variable_name) != _tool_parameters.end()){
    // we are sure the variable_name is already present in the mapping =>
    // we can update it in the following way
    _tool_parameters[variable_name] = new_value;
  }
  else{
    mooseError("Parameter " << variable_name << " was not found in CrowTool type " << _type << ".");
  }
}

void
CrowTools::updateVariable(const char * variable_name, double new_value){
    updateVariable(std::string(variable_name), new_value);
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
getCrowToolVariable(CrowTools & tool,const std::string & variable_name)
{
  return tool.getVariable(variable_name);
}

void
updateCrowToolVariable(CrowTools & tool,const std::string & variable_name, double new_value)
{
  tool.updateVariable(variable_name,new_value);
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

