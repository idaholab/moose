/*
 * CrowToolsContainer.C
 *
 *  Created on: May 29, 2013
 *      Author: alfoa
 */
#include "CrowToolsContainer.h"
#include "CrowTools_min.h"
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

class CrowToolsContainer;

CrowToolsContainer::CrowToolsContainer()
{
}
CrowToolsContainer::~CrowToolsContainer()
{
}

void
CrowToolsContainer::addToolInContainer(const std::string & type, const std::string & name, CrowTools * tool){

  if (_tool_by_name.find(name) == _tool_by_name.end())
    _tool_by_name[name] = tool;
   else
     mooseError("CrowTool with name " << name << " already exists");

   _tool_by_type[type].push_back(tool);

}

std::string
CrowToolsContainer::getType(const char *  ToolAlias){
  return getType(std::string(ToolAlias));
}

std::string
CrowToolsContainer::getType(const std::string ToolAlias){

    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       CrowTools * tool = _tool_by_name.find(ToolAlias)->second;
       std::string type = getCrowToolType(*tool);
       if(type == "CrowToolError"){
         mooseError("Type for CrowTool " << ToolAlias << " not found");
       }
       return type;
    }
    else{
       mooseError("CrowTool " << ToolAlias << " not found in distribution container (get type");
       return "CrowToolError";
    }
}

double
CrowToolsContainer::getVariable(const char * paramName,const char *ToolAlias){
  return getVariable(std::string(paramName),std::string(ToolAlias));
}

double
CrowToolsContainer::getVariable(const std::string paramName,const std::string ToolAlias){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       CrowTools * tool = _tool_by_name.find(ToolAlias)->second;
       return getCrowToolVariable(*tool,paramName);
    }
    mooseError("CrowTool " << ToolAlias << " not found in CrowTools container (get variable)");
    return -1;
}

std::vector<std::string>
CrowToolsContainer::getToolNames(){
  std::vector<std::string> toolsNames;
  for(std::map<std::string, CrowTools *>::iterator it = _tool_by_name.begin(); it!= _tool_by_name.end();it++){
    toolsNames.push_back(it->first);
  }
  return toolsNames;
}

std::vector<std::string>
CrowToolsContainer::getToolVariableNames(const std::string ToolAlias){
  if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
     CrowTools * tool = _tool_by_name.find(ToolAlias)->second;
     return ::getToolVariableNames(*tool);
  }
  mooseError("CrowTool " << ToolAlias << " not found in CrowTools container (getCrowToolVariableNames)");
}
void
CrowToolsContainer::updateVariable(const char * paramName,double newValue,const char *ToolAlias){
  updateVariable(std::string(paramName),newValue,std::string(ToolAlias));
}

void
CrowToolsContainer::updateVariable(const std::string paramName,double newValue,const std::string ToolAlias){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       CrowTools * tool = _tool_by_name.find(ToolAlias)->second;
       updateCrowToolVariable(*tool,paramName,newValue);
    }
    else{
       mooseError("CrowTool " + ToolAlias + " was not found in CrowTools container (update variable)");

    }
}

double
CrowToolsContainer::compute(const char *ToolAlias, double value){
  return compute(std::string(ToolAlias),value);
}

double
CrowToolsContainer::compute(const std::string ToolAlias, double value){
    if(_tool_by_name.find(ToolAlias) != _tool_by_name.end()){
       CrowTools * tool = _tool_by_name.find(ToolAlias)->second;
       return computeCrowTool(*tool,value);
    }
    mooseError("CrowTool " << ToolAlias << " not found in CrowTools container (compute)");
    return -1;
}

CrowToolsContainer & CrowToolsContainer::Instance() {
  if(_instance == NULL){
    _instance = new CrowToolsContainer();
  }
  return *_instance;
}

CrowToolsContainer * CrowToolsContainer::_instance = NULL;

