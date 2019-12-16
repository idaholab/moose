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
class JIntegral;

template <>
InputParameters validParams<JIntegral>();

/**
 * This vectorpostprocessor computes the J-Integral
 *
 */
class JIntegral : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  JIntegral(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real computeQpIntegral(const unsigned int crack_front_point_index, const Real scalar_q, const RealVectorValue & grad_of_scalar_q);
  const CrackFrontDefinition * const _crack_front_definition;
  const MaterialProperty<RankTwoTensor> & _Eshelby_tensor;
  const MaterialProperty<RealVectorValue> * _J_thermal_term_vec;
  bool _convert_J_to_K;
  bool _treat_as_2d;
  /// Vector of all coupled variables
  std::vector<MooseVariableFEBase *> _fe_vars;
  /// FEType object defining order and family of displacement variables
  const FEType & _fe_type;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
  unsigned int _ring_index;

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

  std::vector<Real> _q_curr_elem;
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  unsigned int _qp;

  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _j_integral;
};
