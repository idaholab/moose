//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TWOPHASESTRESSMATERIAL_H
#define TWOPHASESTRESSMATERIAL_H

#include "Material.h"

// Forward Declarations
class TwoPhaseStressMaterial;
class RankTwoTensor;
class RankFourTensor;

template <>
InputParameters validParams<TwoPhaseStressMaterial>();

/**
 * Construct a global strain from the phase strains in a manner that is consistent
 * with the construction of the global elastic energy by DerivativeTwoPhaseMaterial.
 */
class TwoPhaseStressMaterial : public Material
{
public:
  TwoPhaseStressMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  // switching function
  const MaterialProperty<Real> & _h_eta;

  // phase A material properties
  std::string _base_A;
  const MaterialProperty<RankTwoTensor> & _stress_A;
  const MaterialProperty<RankFourTensor> & _dstress_dstrain_A;

  // phase B material properties
  std::string _base_B;
  const MaterialProperty<RankTwoTensor> & _stress_B;
  const MaterialProperty<RankFourTensor> & _dstress_dstrain_B;

  // global material properties
  std::string _base_name;
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankFourTensor> & _dstress_dstrain;
};

#endif // TWOPHASESTRESSMATERIAL_H
