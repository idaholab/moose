//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
