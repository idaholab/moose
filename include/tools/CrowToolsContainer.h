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
  static CrowToolsContainer & Instance();
  /*
   * Function to construct on the fly this class through the action system
   */
  void addToolInContainer(const std::string & type, const std::string & name, CrowTools * tool);

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

  double getVariable(const char * param_name, const char * tool_alias);
  double getVariable(const std::string param_name, const std::string tool_alias);

  void updateVariable(const char * param_name, double new_value, const char * tool_alias);
  void updateVariable(const std::string param_name, double new_value, const std::string tool_alias);

  double compute(const char *tool_alias, double value);
  double compute(const std::string tool_alias, double value);
  std::vector<std::string> getToolNames();
  std::vector<std::string> getToolVariableNames(const std::string tool_alias);


protected:
  std::map < std::string, int > _vector_pos_map;
  /// mapping from tool name and tool itself
  std::map<std::string, CrowTools *> _tool_by_name;
  /// "Buckets" of tools based on their types
  std::map<std::string, std::vector<CrowTools *> > _tool_by_type;

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
