/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBRELAXATIONSTRAININCREMENT_H
#define GBRELAXATIONSTRAININCREMENT_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

class GBRelaxationStrainIncrement;

/**
 * GBRelaxationStrainIncrement computes strain increment due to lattice relaxation at GB
 * Forest et. al. MSMSE 2015
 */
class GBRelaxationStrainIncrement : public DerivativeMaterialInterface<Material>
{
public:
  GBRelaxationStrainIncrement(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  const MaterialProperty<Real> & _prefactor;
  const MaterialProperty<RankTwoTensor> & _gb_normal_tensor;
  MaterialProperty<RankTwoTensor> & _strain_increment;
};

#endif
