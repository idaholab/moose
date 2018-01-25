//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTELINEARELASTICPFFRACTURESTRESS_H
#define COMPUTELINEARELASTICPFFRACTURESTRESS_H

#include "ComputeStressBase.h"

class ComputeLinearElasticPFFractureStress;

template <>
InputParameters validParams<ComputeLinearElasticPFFractureStress>();

/**
 * Phase-field fracture
 * This class computes the stress and energy contribution to fracture
 * Small strain Anisotropic Elastic formulation
 * Stiffness matrix scaled for heterogeneous elasticity property
 */
class ComputeLinearElasticPFFractureStress : public ComputeStressBase
{
public:
  ComputeLinearElasticPFFractureStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();

  const VariableValue & _c;
  /// Small number to avoid non-positive definiteness at or near complete damage
  Real _kdamage;

  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;
};

#endif // COMPUTELINEARELASTICPFFRACTURESTRESS_H
