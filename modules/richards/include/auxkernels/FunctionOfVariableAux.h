#ifndef FUNCTIONOFVARIABLEAUX_H
#define FUNCTIONOFVARIABLEAUX_H

#include "AuxKernel.h"
#include "Function.h"

//Forward Declarations
class FunctionOfVariableAux;

template<>
InputParameters validParams<FunctionOfVariableAux>();

class FunctionOfVariableAux : public AuxKernel
{
public:
  FunctionOfVariableAux(const std::string & name, InputParameters parameters);
  virtual ~FunctionOfVariableAux() {};

protected:
  virtual Real computeValue();

  VariableValue & _t_variable;
  Function & _func;
};

#endif // FUNCTIONOFVARIABLEAUX_H
