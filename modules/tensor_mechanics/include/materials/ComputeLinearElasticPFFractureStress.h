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

  /// Use current value of history variable
  bool _use_current_hist;

  /// Base name of the stress and strain modified to include cracks
  const std::string _uncracked_base_name;

  /// Variable defining the phase field damage parameter
  const VariableValue & _c;

  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  /// Characteristic length, controls damage zone thickness
  const MaterialProperty<Real> & _l;

  // Viscosity, defining how quickly the crack propagates
  const MaterialProperty<Real> & _visco;

  /// Small number to avoid non-positive definiteness at or near complete damage
  Real _kdamage;

  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  MaterialProperty<Real> & _hist;
  const MaterialProperty<Real> & _hist_old;

  /// Property where the value for kappa will be defined
  MaterialProperty<Real> & _kappa;

  /// Property where the value for L will be defined
  MaterialProperty<Real> & _L;
};

#endif // COMPUTELINEARELASTICPFFRACTURESTRESS_H
