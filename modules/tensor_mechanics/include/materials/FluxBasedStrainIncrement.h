/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FLUXBASEDSTRAININCREMENT_H
#define FLUXBASEDSTRAININCREMENT_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

class FluxBasedStrainIncrement;

/**
 * FluxBasedStrainIncrement computes strain increment based on flux (vacancy)
 * Forest et. al. MSMSE 2015
 */
class FluxBasedStrainIncrement : public DerivativeMaterialInterface<Material>
{
public:
  FluxBasedStrainIncrement(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  virtual void computeFluxGradTensor();

  const VariableGradient * _grad_jx;
  bool _has_yflux;
  bool _has_zflux;
  const VariableGradient * _grad_jy;
  const VariableGradient * _grad_jz;

  const VariableValue & _gb;

  MaterialProperty<RankTwoTensor> & _strain_increment;

  RankTwoTensor _flux_grad_tensor;
};

#endif
