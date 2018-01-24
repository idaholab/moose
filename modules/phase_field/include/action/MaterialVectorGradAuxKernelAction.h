//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
