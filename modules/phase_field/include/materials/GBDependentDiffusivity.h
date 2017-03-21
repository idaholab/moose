/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
