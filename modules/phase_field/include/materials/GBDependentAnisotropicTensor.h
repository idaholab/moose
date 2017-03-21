/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBDEPENDENTANISOTROPICTENSOR_H
#define GBDEPENDENTANISOTROPICTENSOR_H

#include "GBDependentTensorBase.h"

class GBDependentAnisotropicTensor;

template <>
InputParameters validParams<GBDependentAnisotropicTensor>();
/**
 * GB dependent anisotropic tensor Ref. Forest, MSMSE, 2015
 */
class GBDependentAnisotropicTensor : public GBDependentTensorBase
{
public:
  GBDependentAnisotropicTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif
