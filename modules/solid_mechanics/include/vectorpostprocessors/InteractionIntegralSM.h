//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"
#include "CrackFrontDefinition.h"
#include "SymmTensor.h"

// Forward Declarations

/**
 * This vectorpostprocessor computes the Interaction Integral
 *
 */
class InteractionIntegralSM : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  InteractionIntegralSM(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  static MooseEnum qFunctionType();
  static MooseEnum sifModeType();

protected:
  Real computeQpIntegral(const std::size_t crack_front_point_index,
                         const Real scalar_q,
                         const RealVectorValue & grad_of_scalar_q);
  void computeAuxFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);
  void computeTFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);
  std::size_t _ndisp;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const std::size_t _crack_front_point_index;
  bool _treat_as_2d;
  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmTensor> & _strain;
  std::vector<const VariableGradient *> _grad_disp;
  const bool _has_temp;
  const VariableGradient & _grad_temp;
  Real _K_factor;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
  std::size_t _ring_index;
  const MaterialProperty<Real> * const _current_instantaneous_thermal_expansion_coef;
  std::vector<Real> _q_curr_elem;
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  Real _kappa;
  Real _shear_modulus;
  Real _r;
  Real _theta;
  unsigned int _qp;

  const enum class QMethod { Geometry, Topology } _q_function_type;

  const enum class PositionType { Angle, Distance } _position_type;

  const enum class SifMethod { KI, KII, KIII, T } _sif_mode;

  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _interaction_integral;
};
