/*
 * CrowToolsContainer.C
 *
 *  Created on: May 29, 2013
 *      Author: alfoa
 */
#include "CrowToolsContainer.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>

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
CrowToolsContainer::addToolInContainer(const std::string & type, const std::string & name, MooseSharedPointer<CrowTools> tool){

  if (_tool_by_name.find(name) == _tool_by_name.end())
    _tool_by_name[name] = tool;
   else
     mooseError("CrowTool with name " << name << " already exists");

   _tool_by_type[type].push_back(tool);

}

std::string
CrowToolsContainer::getType(const char *  tool_alias){
  return getType(std::string(tool_alias));
}

std::string
CrowToolsContainer::getType(const std::string tool_alias){

    if(_tool_by_name.find(tool_alias) != _tool_by_name.end()){
      MooseSharedPointer<CrowTools> tool = _tool_by_name.find(tool_alias)->second;
      std::string type = tool->getType();
      if(type == "CrowToolError"){
        mooseError("Type for CrowTool " << tool_alias << " not found");
      }
      return type;
    }
    else{
       mooseError("CrowTool " << tool_alias << " not found in distribution container (get type");
       return "CrowToolError";
    }
}

double
CrowToolsContainer::getVariable(const char * tool_alias,const char *param_name){
  return getVariable(std::string(tool_alias),std::string(param_name));
}

double
CrowToolsContainer::getVariable(const std::string tool_alias,const std::string param_name){
    if(_tool_by_name.find(tool_alias) != _tool_by_name.end()){
      MooseSharedPointer<CrowTools> tool = _tool_by_name.find(tool_alias)->second;
      return tool->getVariable(param_name);
    }
    mooseError("CrowTool " << tool_alias << " not found in CrowTools container (get variable)");
    return -1;
}

std::vector<std::string>
CrowToolsContainer::getToolNames(){
  std::vector<std::string> toolsNames;
  for(std::map<std::string, MooseSharedPointer<CrowTools> >::iterator it = _tool_by_name.begin(); it!= _tool_by_name.end();it++){
    toolsNames.push_back(it->first);
  }
  return toolsNames;
}

std::vector<std::string>
CrowToolsContainer::getToolVariableNames(const std::string tool_alias){
  if(_tool_by_name.find(tool_alias) != _tool_by_name.end()){
    MooseSharedPointer<CrowTools> tool = _tool_by_name.find(tool_alias)->second;
    return tool->getVariableNames();
  }
  mooseError("CrowTool " << tool_alias << " not found in CrowTools container (getCrowToolVariableNames)");
}

void
CrowToolsContainer::updateVariable(const char * tool_alias ,double new_value, const char * param_name){
  std::cout<< "step 1" << std::endl;
  updateVariable(std::string(tool_alias),new_value,std::string(param_name));
}

void
CrowToolsContainer::updateVariable(const std::string tool_alias, double new_value, const std::string param_name){
    if(_tool_by_name.find(tool_alias) != _tool_by_name.end()){
       std::cout<< "step 2" << std::endl;
       std::cout << "param_name: " << param_name << std::endl;
       std::cout << "new_value: "  << new_value  << std::endl;
       std::cout << "tool_alias: " << tool_alias << std::endl;
       MooseSharedPointer<CrowTools> tool = _tool_by_name.find(tool_alias)->second;
       tool->updateVariable(param_name, new_value);
    }
    else{
       std::cout << "param_name: " << param_name << std::endl;
       std::cout << "new_value: "  << new_value  << std::endl;
       std::cout << "tool_alias: " << tool_alias << std::endl;
       mooseError("CrowTool " + tool_alias + " was not found in CrowTools container (update variable)");
    }
}


double
CrowToolsContainer::compute(const char *tool_alias, double value){
  return compute(std::string(tool_alias),value);
}

double
CrowToolsContainer::compute(const std::string tool_alias, double value){
    if(_tool_by_name.find(tool_alias) != _tool_by_name.end()){
      MooseSharedPointer<CrowTools> tool = _tool_by_name.find(tool_alias)->second;
      return tool->compute(value);
    }
    mooseError("CrowTool " << tool_alias << " not found in CrowTools container (compute)");
    return -1;
}

CrowToolsContainer & CrowToolsContainer::Instance() {
  if(_instance == NULL){
    _instance = new CrowToolsContainer();
  }
  return *_instance;
}

CrowToolsContainer * CrowToolsContainer::_instance = NULL;

