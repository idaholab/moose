/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATVECREALGRADAUXKERNELACTION_H
#define MATVECREALGRADAUXKERNELACTION_H

#include "Action.h"

class MatVecRealGradAuxKernelAction: public Action
{
public:
  MatVecRealGradAuxKernelAction(const InputParameters & params);

  virtual void act();

private:
  std::string _var_name_base;
};

template<>
InputParameters validParams<MatVecRealGradAuxKernelAction>();

#endif //MATVECREALGRADAUXKERNELACTION_H
