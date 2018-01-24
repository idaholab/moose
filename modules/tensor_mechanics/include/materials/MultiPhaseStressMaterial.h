//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIPHASESTRESSMATERIAL_H
#define MULTIPHASESTRESSMATERIAL_H

#include "Material.h"

// Forward Declarations
class MultiPhaseStressMaterial;
class RankTwoTensor;
class RankFourTensor;

template <>
InputParameters validParams<MultiPhaseStressMaterial>();

/**
 * Construct a global strain from the phase strains in a manner that is consistent
 * with the construction of the global elastic energy by DerivativeMultiPhaseMaterial.
 */
class MultiPhaseStressMaterial : public Material
{
public:
  MultiPhaseStressMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// switching function name list
  std::vector<MaterialPropertyName> _h_list;

  /// number of phases handled by this material
  unsigned int _n_phase;

  /// switching functions
  std::vector<const MaterialProperty<Real> *> _h_eta;

  // phase material properties
  std::vector<std::string> _phase_base;
  std::vector<const MaterialProperty<RankTwoTensor> *> _phase_stress;
  std::vector<const MaterialProperty<RankFourTensor> *> _dphase_stress_dstrain;

  // global material properties
  std::string _base_name;
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankFourTensor> & _dstress_dstrain;
};

#endif // MULTIPHASESTRESSMATERIAL_H
