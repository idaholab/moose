/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef FUNCTIONOFVARIABLEAUX_H
#define FUNCTIONOFVARIABLEAUX_H

#include "AuxKernel.h"
#include "Function.h"

//Forward Declarations
class FunctionOfVariableAux;

template<>
InputParameters validParams<FunctionOfVariableAux>();

/**
 * Aux variable that is a function of another variable
 * The function is specified and the variable is passed into
 * its "t" slot.  This is a bit bodgy but i find it quite useful
 */
class FunctionOfVariableAux : public AuxKernel
{
public:
  FunctionOfVariableAux(const std::string & name, InputParameters parameters);
  virtual ~FunctionOfVariableAux() {};

protected:
  virtual Real computeValue();

  /// the variable that you want to take a function of
  VariableValue & _t_variable;

  /// the function that you want to use
  Function & _func;
};

#endif // FUNCTIONOFVARIABLEAUX_H
