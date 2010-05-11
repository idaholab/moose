#ifndef GENERICVARIABLEBLOCK_H
#define GENERICVARIABLEBLOCK_H

#include "InputParameters.h"
#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;

template<>
InputParameters validParams<GenericVariableBlock>();

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(std::string name, MooseSystem & moose_system, InputParameters params);

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
