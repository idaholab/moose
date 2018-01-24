/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATVECREALGRADAUXKERNELACTION_H
#define MATVECREALGRADAUXKERNELACTION_H

#include "Action.h"

class MatVecRealGradAuxKernelAction : public Action
{
public:
  MatVecRealGradAuxKernelAction(const InputParameters & params);

  virtual void act();

protected:
  const AuxVariableName _div_var;
  const std::vector<MaterialPropertyName> _prop;
  const MaterialPropertyName _div_prop;
};

template <>
InputParameters validParams<MatVecRealGradAuxKernelAction>();

#endif // MATVECREALGRADAUXKERNELACTION_H
