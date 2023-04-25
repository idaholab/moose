//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarUserObject.h"

/**
 * Creates dof object to weighted gap map
 */
class WeightedGapUserObject : public MortarUserObject
{
public:
  static InputParameters validParams();

  WeightedGapUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void initialSetup() override;

  /**
   * Get the degree of freedom to weighted gap information
   */
  const std::unordered_map<const DofObject *, std::pair<ADReal, Real>> & dofToWeightedGap() const;

  /**
   * @return The contact force at quadrature points on the mortar segment
   */
  virtual const ADVariableValue & contactPressure() const = 0;

  /**
   * @param node Node pointer
   * @return The normal contact pressure at the node
   */
  virtual Real getNormalContactPressure(const Node * const /*node*/) const
  {
    mooseError("Not available in base class.");
  }

  /**
   * @param node Node pointer
   * @return The normal contact pressure at the node
   */
  virtual Real getNormalWeightedGap(const Node * const /*node*/) const;

  /**
   * @param node Node pointer
   * @param component Component of the frictional pressure vector
   * @return The frictional contact pressure at the node
   */
  virtual Real getFrictionalContactPressure(const Node * const /*node*/,
                                            const unsigned int /*component*/) const
  {
    mooseError("Not available in base class.");
  }

  /**
   * @param node Node pointer
   * @param component Component of the local slip vector
   * @return The accumulated slip at the node
   */
  virtual Real getAccumulatedSlip(const Node * const /*node*/,
                                  const unsigned int /*component*/) const
  {
    mooseError("Not available in base class.");
  }

  /**
   * @param node Node pointer
   * @param component Component of the local slip vector
   * @return The tangential velocity at the node with local components
   */
  virtual Real getTangentialVelocity(const Node * const /*node*/,
                                     const unsigned int /*component*/) const
  {
    mooseError("Not available in base class.");
  }

protected:
  /**
   * Computes properties that are functions only of the current quadrature point (\p _qp), e.g.
   * indepedent of shape functions
   */
  virtual void computeQpProperties();

  /**
   * Computes properties that are functions both of \p _qp and \p _i, for example the weighted gap
   */
  virtual void computeQpIProperties();

  /**
   * @return The test function associated with the weighted gap
   */
  virtual const VariableTestValue & test() const = 0;

  /**
   * @return Whether the gap constraint will be enforced solely by the owner of the weighted gap or
   * will be enforced in a distributed way (like in a penalty method)
   */
  virtual bool constrainedByOwner() const = 0;

  /// The base finite element problem
  FEProblemBase & _fe_problem;

  /// The value of the gap at the current quadrature point
  ADReal _qp_gap;

  /// The value of the LM at the current quadrature point
  Real _qp_factor;

  /// Whether the dof objects are nodal; if they're not, then they're elemental
  const bool _nodal;

  /// The x displacement variable
  const MooseVariable * const _disp_x_var;
  /// The y displacement variable
  const MooseVariable * const _disp_y_var;
  /// For 2D mortar contact no displacement will be specified, so const pointers used
  const bool _has_disp_z;
  /// The z displacement variable
  const MooseVariable * const _disp_z_var;

  /// x-displacement on the secondary face
  const ADVariableValue & _secondary_disp_x;
  /// x-displacement on the primary face
  const ADVariableValue & _primary_disp_x;
  /// y-displacement on the secondary face
  const ADVariableValue & _secondary_disp_y;
  /// y-displacement on the primary face
  const ADVariableValue & _primary_disp_y;
  /// z-displacement on the secondary face
  const ADVariableValue * const _secondary_disp_z;
  /// z-displacement on the primary face
  const ADVariableValue * const _primary_disp_z;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// Vector for computation of weighted gap with nodal normals
  ADRealVectorValue _qp_gap_nodal;

  /// A map from node to weighted gap and normalization (if requested)
  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_weighted_gap;

  /// A pointer members that can be used to help avoid copying ADReals
  const ADReal * _weighted_gap_ptr = nullptr;
  const Real * _normalization_ptr = nullptr;

  /// A pointer to the test function associated with the weighted gap. We have this member so that
  /// we don't do virtual calls during inner quadrature-point/test-function loops
  const VariableTestValue * _test = nullptr;

  /// Whether the weighted gap is associated with nodes or elements (like for a CONSTANT MONOMIAL
  /// Lagrange multiplier). We have this member so that we don't do virtual calls during inner
  /// quadrature-point/test-function loops
  bool _is_weighted_gap_nodal = true;

  /// Quadrature point index for the mortar segments
  unsigned int _qp = 0;

  /// Test function index
  unsigned int _i = 0;
};

inline const std::unordered_map<const DofObject *, std::pair<ADReal, Real>> &
WeightedGapUserObject::dofToWeightedGap() const
{
  return _dof_to_weighted_gap;
}
