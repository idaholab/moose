//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GBDEPENDENTDIFFUSIVITY_H
#define GBDEPENDENTDIFFUSIVITY_H

#include "GBDependentTensorBase.h"

class GBDependentDiffusivity;

template <>
InputParameters validParams<GBDependentDiffusivity>();
/**
 * GB dependent diffusivity Ref. Forest, MSMSE, 2015
 */
class GBDependentDiffusivity : public GBDependentTensorBase
{
public:
  GBDependentDiffusivity(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif
