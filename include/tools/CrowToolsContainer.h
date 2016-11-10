#ifndef CROWTOOLSCONTAINER_H
#define CROWTOOLSCONTAINER_H

#include <iostream>
#include <vector>
#include <map>
#include "CrowTools.h"

class CrowToolsContainer;

class InputParameters;

class CrowToolsContainer
{
public:
  static CrowToolsContainer & instance();
  /*
   * Function to construct on the fly this class through the action system
   */
  void addToolInContainer(const std::string & type, const std::string & name, MooseSharedPointer<CrowTools> tool);

  bool
  isEmpty()
  {
    return _tool_by_name.empty();
  };
  /**
   * Function to get the enum of the distribution called tool_alias
   * @param  tool_alias, alias of the crow tool from which retrieving the parameter
   */
  std::string getType (const char * tool_alias);
  std::string getType (const std::string tool_alias);

  double getVariable(const char * tool_alias, const char * param_name);
  double getVariable(const std::string tool_alias, const std::string param_name);

  //void updateVariable(const char * param_name, double new_value, const char * tool_alias);
  //void updateVariable(const std::string param_name, double new_value, const std::string tool_alias);
  void updateVariable(const std::string tool_alias, double new_value, const std::string param_name);
  void updateVariable(const char * tool_alias ,double new_value, const char * param_name);

  double compute(const char *tool_alias, double value);
  double compute(const std::string tool_alias, double value);
  std::vector<std::string> getToolNames();
  std::vector<std::string> getToolVariableNames(const std::string tool_alias);


protected:
  std::map < std::string, int > _vector_pos_map;
  /// mapping from tool name and tool itself
  std::map<std::string, MooseSharedPointer<CrowTools> > _tool_by_name;
  /// "Buckets" of tools based on their types
  std::map<std::string, std::vector<MooseSharedPointer<CrowTools> > > _tool_by_type;

  /**
   * Constructor(empty)
   */
  CrowToolsContainer();
  /**
   * Destructor
   */
  virtual ~CrowToolsContainer();
  static CrowToolsContainer * _instance; // = NULL
};

#endif /* CROWTOOLSCONTAINER_H */
