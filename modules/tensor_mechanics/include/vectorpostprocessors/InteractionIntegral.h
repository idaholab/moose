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
 * This vectorpostprocessor computes the Interaction Integral, which is
 * used to compute various fracture mechanics parameters at a crack tip,
 * including KI, KII, KIII, and the T stress.
 */
template <bool is_ad>
class InteractionIntegralTempl : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  InteractionIntegralTempl(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  /**
   * Get the MooseEnum defining the options for how the q function is defined
   * @return MooseEnum with options for q function
   */
  static MooseEnum qFunctionType();

  /**
   * Get the MooseEnum defining the options for the integral to be computed
   * @return MooseEnum with options for the integral to be computed
   */
  static MooseEnum sifModeType();

protected:
  /**
   * Compute the contribution of a specific quadrature point to a fracture integral
   * for a point on the crack front.
   * @param crack_front_point_index Index to the crack front point
   * @param scalar_q Magnitude of the q vector at the quadrature point
   * @param grad_of_scalar_q Gradient of the magnitude of the q vector at the
   *                         quadrature point
   * @return Contribution of this quadrature point to the integral
   */
  Real computeQpIntegral(const std::size_t crack_front_point_index,
                         const Real scalar_q,
                         const RealVectorValue & grad_of_scalar_q);

  /**
   * Compute the auxiliary fields, including the auxiliary stress and the
   * gradient of the auxiliary displacement for the current point (as
   * defined by the combination of _r and _theta)
   * @param aux_stress Auxiliary stress -- computed in this method
   * @param grad_disp Gradient of auxiliary displacement -- computed in this method
   * @param grad_disp Auxiliary strain (FGM) -- computed in this method
   * @param aux_disp Auxiliary displacement -- computed in this method only for axisymmetric (RZ)
   meshes

   */
  void computeAuxFields(RankTwoTensor & aux_stress,
                        RankTwoTensor & grad_disp,
                        RankTwoTensor & aux_strain,
                        RankTwoTensor & aux_disp);
  /**
   * Compute the auxiliary fields, including the auxiliary stress and the
   * gradient of the auxiliary displacement for the current point (as
   * defined by the combination of _r and _theta) for the T stress
   * @param aux_stress Auxiliary stress -- computed in this method
   * @param grad_disp Gradient of auxiliary displacement -- computed in this method
   */
  void computeTFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);

  /// Number of displacement components
  std::size_t _ndisp;
  /// Pointer to the crack front definition object
  const CrackFrontDefinition * const _crack_front_definition;
  /// Whether to treat a 3D model as 2D for computation of fracture integrals
  bool _treat_as_2d;
  /// Reference to the stress tensor computed by the material models
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;
  /// Reference to the strain tensor computed by the material models
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _strain;
  /// Vector of all coupled variables
  std::vector<MooseVariableFEBase *> _fe_vars;
  /// FEType object defining order and family of displacement variables
  const FEType & _fe_type;
  /// Displacement variables
  std::vector<const VariableValue *> _disp;
  /// Gradient of displacements
  std::vector<const VariableGradient *> _grad_disp;
  /// Whether the temperature variable is coupled
  const bool _has_temp;
  /// Gradient of temperature
  const VariableGradient & _grad_temp;
  /// Spatial derivative of the youngs modulus in the crack direction
  const MaterialProperty<Real> * _functionally_graded_youngs_modulus_crack_dir_gradient;
  /// Spatial elasticity modulus variable for FGM
  const MaterialProperty<Real> * _functionally_graded_youngs_modulus;
  /// Conversion factor applied to convert interaction integral to stress intensity factor K
  Real _K_factor;
  /// Whether the crack plane is also a symmetry plane in the model
  bool _has_symmetry_plane;
  /// Poisson's ratio of the material
  Real _poissons_ratio;
  /// Young's modulus of the material
  Real _youngs_modulus;
  /// Whether to consider interaction integral and material properties for a crack in functionally graded material
  const bool _fgm_crack;
  /// Index of the ring for the integral computed by this object
  std::size_t _ring_index;
  /// Derivative of the total eigenstrain with respect to temperature
  const GenericMaterialProperty<RankTwoTensor, is_ad> * const _total_deigenstrain_dT;
  /// Vector of q function values for the nodes in the current element
  std::vector<Real> _q_curr_elem;
  /// Vector of shape function values for the current element
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  /// Vector of gradients of shape function values for the current element
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  /// Kappa Lame constant
  Real _kappa;
  /// Shear modulus
  Real _shear_modulus;
  /// Radial distance from the current point to the crack front
  Real _r;
  /// Angle of current point relative to the crack front
  Real _theta;
  /// Current quadrature point index
  unsigned int _qp;

  /// Enum used to select the method used to compute the q function used
  /// in the fracture integrals
  const enum class QMethod { Geometry, Topology } _q_function_type;

  /// Enum used to define how the distance along the crack front is
  /// measured (angle or distance)
  const enum class PositionType { Angle, Distance } _position_type;

  /// Enum used to select the type of integral to be performed
  const enum class SifMethod { KI, KII, KIII, T } _sif_mode;

  /// Vectors computed by this VectorPostprocessor:
  /// x,y,z coordinates, position of nodes along crack front, and
  /// interaction integral
  ///@{
  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _interaction_integral;
  ///@}

  /// Pointers to optionally-used eigenstrain gradient and body force
  ///@{
  const GenericMaterialProperty<RankThreeTensor, is_ad> * _eigenstrain_gradient;
  const MaterialProperty<RealVectorValue> * _body_force;
  ///@}
};

typedef InteractionIntegralTempl<false> InteractionIntegral;
typedef InteractionIntegralTempl<true> ADInteractionIntegral;
