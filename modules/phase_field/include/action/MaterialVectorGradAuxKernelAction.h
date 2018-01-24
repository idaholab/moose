/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATERIALVECTORGRADAUXKERNELACTION_H
#define MATERIALVECTORGRADAUXKERNELACTION_H

#include "MaterialVectorAuxKernelAction.h"

class MaterialVectorGradAuxKernelAction : public MaterialVectorAuxKernelAction
{
public:
  MaterialVectorGradAuxKernelAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<MaterialVectorGradAuxKernelAction>();

#endif // MATERIALVECTORGRADAUXKERNELACTION_H
