#ifndef GENERICVARIABLEBLOCK_H_
#define GENERICVARIABLEBLOCK_H_

#include "InputParameters.h"
#include "ParserBlock.h"

class GenericVariableBlock: public ParserBlock
{
public:
  GenericVariableBlock(const std::string & name, InputParameters params);

  virtual void execute();

  bool restartRequired() const;
  bool autoResizeable() const;
  std::pair<std::string, unsigned int> initialValuePair() const;

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
  unsigned int _timestep_to_read;
};

template<>
InputParameters validParams<GenericVariableBlock>();


#endif //GENERICVARIABLEBLOCK_H
