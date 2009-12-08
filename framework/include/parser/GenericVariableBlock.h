#ifndef GENERICVARIABLEBLOCK_H
#define GENERICVARIABLEBLOCK_H

#include "InputParameters.h"
#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;

template<>
InputParameters validParams<GenericVariableBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<int>("initial_from_file_timestep", 2, "Gives the timestep for which to read a solution from a file for a given variable");
  params.addParam<std::string>("initial_from_file_var", "Gives the name of a variable for which to read an initial condition from a mesh file");

  return params;
}

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

  bool restartRequired() const;
  bool autoResizeable() const;
  std::pair<std::string, unsigned int> initialValuePair() const;

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
  unsigned int _timestep_to_read;
};

#endif //GENERICVARIABLEBLOCK_H
