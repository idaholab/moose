//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarUserObject.h"

#include <unordered_map>

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
  virtual Real getNormalContactPressure(const Node * const /*node*/) const = 0;

  /**
   * @param node Node pointer
   * @return The normal gap at the node
   */
  virtual Real getNormalGap(const Node * const /*node*/) const;

  /**
   * Compute physical gap from integration gap quantity
   */
  Real physicalGap(const std::pair<ADReal, Real> & gap) const
  {
    return MetaPhysicL::raw_value(gap.first) / gap.second;
  }

  /**
   * Node-based scaling factor kappa_j (Popp et al. 2013, eq. 36): the mean over node j's adjacent
   * secondary elements of its covered shape-function-integral fraction; 1 at full coverage, -> 0 as
   * node j drops. Rescales the multiplier (zhat_j = kappa_j lambda_j) for conditioning; 1 if
   * absent. Non-AD, so its linearization (eq. 38) is omitted, affecting only the near-drop Newton
   * rate.
   * @return kappa_j at the node (1 if absent)
   */
  Real nodalScale(const DofObject * const dof) const
  {
    return findValue(_dof_to_nodal_scale, dof, Real(1));
  }

  /// @return Whether node-based Lagrange-multiplier scaling was requested on this user object.
  /// Consumed by mortar constraints that do not (yet) support the scaling so they can error out.
  bool usesNodalScaling() const { return _use_nodal_scaling; }

  /// @return Whether this user object actually applies the node-based scaling. Only the Lagrange
  /// multiplier formulation (LMWeightedGapUserObject) does; other formulations (penalty, plain
  /// weighted velocities) would silently ignore the option, so they reject it (see initialSetup()).
  virtual bool nodalScalingApplied() const { return false; }

  ADReal adPhysicalGap(const std::pair<ADReal, Real> & gap) const { return gap.first / gap.second; }

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
   * Full coordinate-weighted integral int_e N_j per local node on the secondary lower-d element
   * (exact in RZ/spherical and on warped elements), via a temporary FE and cached: the denominator
   * of kappa_j (Popp 2013 eq. 36).
   * @param elem The secondary lower-dimensional element
   * @return Per-local-node full shape-function integrals
   */
  const std::vector<Real> & fullNodalIntegrals(const Elem * elem);

  /**
   * @return The test function associated with the weighted gap
   */
  virtual const VariableTestValue & test() const = 0;

  /**
   * @return Whether the gap constraint will be enforced solely by the owner of the weighted gap or
   * will be enforced in a distributed way (like in a penalty method)
   */
  virtual bool constrainedByOwner() const = 0;

  /**
   * Find a value in a map or return a default if the key doesn't exist
   */
  template <typename K, typename V>
  V
  findValue(const std::unordered_map<K, V> & map, const K & key, const V & default_value = 0) const
  {
    const auto it = map.find(key);
    if (it == map.end())
      return default_value;
    return it->second;
  }

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

  /// Vector for computation of relative displacement (determines mixity ratio in interface problems)
  ADRealVectorValue _qp_displacement_nodal;

  /// Whether to apply the Popp et al. (2013) node-based Lagrange-multiplier scaling (kappa_j) for
  /// improved conditioning of partially covered (edge-dropping) secondary elements
  const bool _use_nodal_scaling;

  /// A map from node to weighted gap and normalization (if requested)
  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_weighted_gap;

  /// Per-node numerator of kappa_j (Popp 2013 eq. 36), summed over adjacent secondary elements;
  /// finalize() divides by the adjacency count. Only when _use_nodal_scaling.
  std::unordered_map<const DofObject *, Real> _dof_to_covered_fraction_sum;

  /// A map from node to its node-based scaling factor kappa_j (see nodalScale()). Only populated
  /// when _use_nodal_scaling is true.
  std::unordered_map<const DofObject *, Real> _dof_to_nodal_scale;

  /// Cache of the per-node full integrals int_e N_j (see fullNodalIntegrals()), keyed by element id;
  /// cleared each evaluation in initialize() since the displaced geometry changes.
  std::unordered_map<dof_id_type, std::vector<Real>> _elem_to_full_nodal_integral;

  /// A map from node to weighted displacements
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_weighted_displacements;

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
