#ifndef CROWTOOLS_H
#define CROWTOOLS_H

#include "InterpolationFunctions.h"
#include "MooseObject.h"

class CrowTools;

template<>
InputParameters validParams<CrowTools>();

class CrowTools : public MooseObject
{
public:
  //> constructor for built-in crow tools
  CrowTools(const InputParameters & parameters);

  virtual ~CrowTools();
  /*
   * All variables except status ones
   */
  double  getVariable(std::string variable_name);                     ///< getVariable from mapping
  void updateVariable(const std::string variable_name, double new_value);  ///< update variable into the mapping//   void updateVariable(const char variable_name, double & new_value);
  void updateVariable(const char * variable_name, double  new_value);
  virtual double compute(double value);
  std::string & getType();        ///< Get CrowTool type
   std::vector<std::string> getVariableNames(); ///< Get variable Names

protected:
  std::string _type;               ///< CrowTools' type
  std::map <std::string,double> _tool_parameters  ;  ///< CrowTools' parameters
};

#endif /* CROWTOOLS_H */
