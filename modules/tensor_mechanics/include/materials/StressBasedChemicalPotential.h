//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STRESSBASEDCHEMICALPOTENTIAL_H
#define STRESSBASEDCHEMICALPOTENTIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

class StressBasedChemicalPotential;

template <>
InputParameters validParams<StressBasedChemicalPotential>();

/**
 * StressBasedChemicalPotential computes chemical potential based on stress and
 * a direction tensor
 * Forest et. al. MSMSE 2015
 */
class StressBasedChemicalPotential : public DerivativeMaterialInterface<Material>
{
public:
  StressBasedChemicalPotential(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  MaterialProperty<Real> & _chemical_potential;
  MaterialProperty<Real> * _dchemical_potential;

  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RealTensorValue> & _direction_tensor;
  const MaterialProperty<Real> & _prefactor;
  const MaterialProperty<Real> * _dprefactor_dc;
  bool _has_coupled_c;
};

#endif
