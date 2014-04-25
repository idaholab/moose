/*
 * RavenToolsContainer.h
 *
 *  Created on: May 30, 2013
 *      Author: alfoa
 */

#ifndef RAVENTOOLSCONTAINER_H_
#define RAVENTOOLSCONTAINER_H_

#include <iostream>
#include <vector>
#include <map>
#include "RavenTools.h"
#include "RavenTools_min.h"

using namespace std;

class RavenToolsContainer;

class InputParameters;

class RavenToolsContainer{
     public:
     static RavenToolsContainer & Instance();
     /*
      * Function to construct on the fly this class through the action system
      */
     //void addToolInContainer(const std::string & type, const std::string & name, InputParameters params);
     void addToolInContainer(const std::string & type, const std::string & name, RavenTools * tool);


     bool isEmpty(){return _tool_by_name.empty();};
     /*
      * Function to get the enum of the distribution called ToolAlias
      * @  ToolAlias, alias of the raven tool from which retrieving the parameter
      */
     std::string getType (const char * ToolAlias);
     std::string getType (const std::string ToolAlias);

     double getVariable(const char * paramName,const char * ToolAlias);
     double getVariable(const std::string paramName,const std::string ToolAlias);

     void updateVariable(const char * paramName,double newValue,const char * ToolAlias);
     void updateVariable(const std::string paramName,double newValue,const std::string ToolAlias);

     double compute(const char *ToolAlias, double value);
     double compute(const std::string ToolAlias, double value);
     std::vector<std::string> getRavenToolNames();
     //std::vector<std::string> getRavenToolVariableNames(char * ToolAlias);
     std::vector<std::string> getRavenToolVariableNames(const std::string ToolAlias);


     protected:
     std::map < std::string, int > _vector_pos_map;
     /// mapping from tool name and tool itself
     std::map<std::string, RavenTools *> _tool_by_name;
     /// "Buckets" of tools based on their types
     std::map<std::string, std::vector<RavenTools *> > _tool_by_type;

     /*
      * Constructor(empty)
      */
     RavenToolsContainer();
     /*
      * Destructor
      */
     virtual ~RavenToolsContainer();
     static RavenToolsContainer * _instance; // = NULL
};

#endif /* RAVENTOOLSCONTAINER_H_ */
