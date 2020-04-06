//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * Computes energy and modifies the stress for phase field fracture. Can be used with any
 * constitutive model or elastic symmetry.
 */
class ComputeCrackedStress : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeCrackedStress(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();
  /// Base name of the stress after being modified to include cracks
  const std::string _base_name;

  /// Base name of the uncracked stress and strain
  const std::string _uncracked_base_name;

  /// Indicator if finite strain model is used, to determine if mechanical_strain or elastic_strain should be used
  bool _finite_strain_model;

  /// Use current value of history variable
  bool _use_current_hist;

  /// Mechanical_strain if finite_strain_model = false, otherwise elastic_strain
  const MaterialProperty<RankTwoTensor> & _strain;

  /// Uncracked stress calculated by another material
  const MaterialProperty<RankTwoTensor> & _uncracked_stress;

  /// Uncracked Jacobian_mult calculated by another material
  const MaterialProperty<RankFourTensor> & _uncracked_Jacobian_mult;

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

  /// Stress being computed by this kernel
  MaterialProperty<RankTwoTensor> & _stress;

  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  /// history variable storing the maximum positive deformation energy
  MaterialProperty<Real> & _hist;
  const MaterialProperty<Real> & _hist_old;

  /// derivative of stress w.r.t. strain (_dstress_dstrain)
  MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// Property where the value for kappa will be defined
  MaterialProperty<Real> & _kappa;

  /// Property where the value for L will be defined
  MaterialProperty<Real> & _L;
};
