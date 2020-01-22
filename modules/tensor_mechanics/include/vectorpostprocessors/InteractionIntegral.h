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

// Forward Declarations
class InteractionIntegral;

template <>
InputParameters validParams<InteractionIntegral>();

/**
 * This vectorpostprocessor computes the Interaction Integral
 *
 */
class InteractionIntegral : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  InteractionIntegral(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  static MooseEnum qFunctionType();
  static MooseEnum sifModeType();

protected:
  Real computeQpIntegral(const unsigned int crack_front_point_index,
                         const Real scalar_q,
                         const RealVectorValue & grad_of_scalar_q);
  void computeAuxFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);
  void computeTFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);
  unsigned int _ndisp;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
  const MaterialProperty<RankTwoTensor> * _stress;
  const MaterialProperty<RankTwoTensor> * _strain;
  /// Vector of all coupled variables
  std::vector<MooseVariableFEBase *> _fe_vars;
  /// FEType object defining order and family of displacement variables
  const FEType & _fe_type;
  std::vector<const VariableGradient *> _grad_disp;
  const bool _has_temp;
  const VariableGradient & _grad_temp;
  Real _K_factor;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
  unsigned int _ring_index;
  const MaterialProperty<RankTwoTensor> * _total_deigenstrain_dT;
  std::vector<Real> _q_curr_elem;
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  Real _kappa;
  Real _shear_modulus;
  Real _r;
  Real _theta;
  unsigned int _qp;

  enum class QMethod
  {
    Geometry,
    Topology
  };

  const QMethod _q_function_type;

  enum class PositionType
  {
    Angle,
    Distance
  };

  const PositionType _position_type;

  enum class SifMethod
  {
    KI,
    KII,
    KIII,
    T
  };

  const SifMethod _sif_mode;

  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _interaction_integral;
};
