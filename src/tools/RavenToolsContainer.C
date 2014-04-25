/*
 * RavenToolsContainer.C
 *
 *  Created on: May 29, 2013
 *      Author: alfoa
 */
#include "RavenToolsContainer.h"
#include "RavenTools_min.h"
//#include "RavenTools.h"
#include <iostream>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <vector>
#include <map>

using namespace std;

#ifndef mooseError
#define mooseError(msg) { std::cerr << "\n\n" << msg << "\n\n"; }
#endif

class RavenToolsContainer;

RavenToolsContainer::RavenToolsContainer()
{
}
RavenToolsContainer::~RavenToolsContainer()
{
}

void
RavenToolsContainer::addToolInContainer(const std::string & type, const std::string & name, RavenTools * tool){

  if (_tool_by_name.find(name) == _tool_by_name.end())
    _tool_by_name[name] = tool;
   else
     mooseError("RavenTool with name " << name << " already exists");

   _tool_by_type[type].push_back(tool);

}

std::string
RavenToolsContainer::getType(const char *  ToolAlias){
  return getType(std::string(ToolAlias));
}

std::string
RavenToolsContainer::getType(const std::string ToolAlias){

    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       RavenTools * tool = _tool_by_name.find(ToolAlias)->second;
       std::string type = getRavenToolType(*tool);
       if(type == "RavenToolError"){
         mooseError("Type for RavenTool " << ToolAlias << " not found");
       }
       return type;
    }
    else{
       mooseError("RavenTool " << ToolAlias << " not found in distribution container (get type");
       return "RavenToolError";
    }
}

double
RavenToolsContainer::getVariable(const char * paramName,const char *ToolAlias){
  return getVariable(std::string(paramName),std::string(ToolAlias));
}

double
RavenToolsContainer::getVariable(const std::string paramName,const std::string ToolAlias){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       RavenTools * tool = _tool_by_name.find(ToolAlias)->second;
       return getRavenToolVariable(*tool,paramName);
    }
    mooseError("RavenTool " << ToolAlias << " not found in RavenTools container (get variable)");
    return -1;
}

std::vector<std::string>
RavenToolsContainer::getRavenToolNames(){
  std::vector<std::string> toolsNames;
  for(std::map<std::string, RavenTools *>::iterator it = _tool_by_name.begin(); it!= _tool_by_name.end();it++){
    toolsNames.push_back(it->first);
  }
  return toolsNames;
}

std::vector<std::string>
RavenToolsContainer::getRavenToolVariableNames(const std::string ToolAlias){
  if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
     RavenTools * tool = _tool_by_name.find(ToolAlias)->second;
     return getToolVariableNames(*tool);
  }
  mooseError("RavenTool " << ToolAlias << " not found in RavenTools container (getRavenToolVariableNames)");
}
void
RavenToolsContainer::updateVariable(const char * paramName,double newValue,const char *ToolAlias){
  updateVariable(std::string(paramName),newValue,std::string(ToolAlias));
}

void
RavenToolsContainer::updateVariable(const std::string paramName,double newValue,const std::string ToolAlias){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       RavenTools * tool = _tool_by_name.find(ToolAlias)->second;
       updateRavenToolVariable(*tool,paramName,newValue);
    }
    else{
       mooseError("RavenTool " + ToolAlias + " was not found in RavenTools container (update variable)");

    }
}

double
RavenToolsContainer::compute(const char *ToolAlias, double value){
  return compute(std::string(ToolAlias),value);
}

double
RavenToolsContainer::compute(const std::string ToolAlias, double value){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       RavenTools * tool = _tool_by_name.find(ToolAlias)->second;
       return computeRavenTool(*tool,value);
    }
    mooseError("RavenTool " << ToolAlias << " not found in RavenTools container (compute)");
    return -1;
}

RavenToolsContainer & RavenToolsContainer::Instance() {
  if(_instance == NULL){
    _instance = new RavenToolsContainer();
  }
  return *_instance;
}

RavenToolsContainer * RavenToolsContainer::_instance = NULL;

