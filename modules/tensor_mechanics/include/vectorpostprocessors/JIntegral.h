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

/**
 * This vectorpostprocessor computes the J-Integral, which is a measure of
 * the strain energy release rate at a crack tip, which can be used as a
 * criterion for fracture growth.
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
  /**
   * Compute the contribution of a specific quadrature point to the J integral
   * for a point on the crack front.
   * @param crack_front_point_index Index to the crack front point
   * @param scalar_q Magnitude of the q vector at the quadrature point
   * @param grad_of_scalar_q Gradient of the magnitude of the q vector at the
   *                         quadrature point
   * @return Contribution of this quadrature point to the J integral
   */
  Real computeQpIntegral(const std::size_t crack_front_point_index,
                         const Real scalar_q,
                         const RealVectorValue & grad_of_scalar_q);
  const CrackFrontDefinition * const _crack_front_definition;
  /// Enum defining the type of integral to be computed
  const enum class IntegralType { JIntegral, CIntegral, KFromJIntegral } _integral;

  const MaterialProperty<RealVectorValue> * const _J_thermal_term_vec;

  /// Eshelby tensor for J integral and K stress intensity factor
  const MaterialProperty<RankTwoTensor> * _Eshelby_tensor;

  /// Eshelby tensor rate for computing the C(t) integral
  const MaterialProperty<RankTwoTensor> * _Eshelby_tensor_dissipation;

  /// Whether to treat a 3D model as 2D for computation of fracture integrals
  bool _treat_as_2d;
  /// Vector of all coupled variables
  std::vector<MooseVariableFEBase *> _fe_vars;
  /// FEType object defining order and family of displacement variables
  const FEType & _fe_type;
  /// Whether the crack plane is also a symmetry plane in the model
  bool _has_symmetry_plane;
  /// Poisson's ratio of the material
  Real _poissons_ratio;
  /// Young's modulus of the material
  Real _youngs_modulus;
  /// Index of the ring for the integral computed by this object
  std::size_t _ring_index;
  /// Enum used to select the method used to compute the q function used
  /// in the fracture integrals
  const enum class QMethod { Geometry, Topology } _q_function_type;

  /// Enum used to define how the distance along the crack front is
  /// measured (angle or distance)
  const enum class PositionType { Angle, Distance } _position_type;

  /// Vector of q function values for the nodes in the current element
  std::vector<Real> _q_curr_elem;
  /// Vector of shape function values for the current element
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  /// Vector of gradients of shape function values for the current element
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  /// Current quadrature point index
  unsigned int _qp;
  /// Vectors computed by this VectorPostprocessor:
  /// x,y,z coordinates, position of nodes along crack front, and
  /// interaction integral
  ///@{
  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _j_integral;
  ///@}
};
