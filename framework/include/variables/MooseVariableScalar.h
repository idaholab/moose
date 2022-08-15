//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableBase.h"
#include "SystemBase.h"

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class Assembly;
class TimeIntegrator;

/**
 * Class for scalar variables (they are different).
 */
class MooseVariableScalar : public MooseVariableBase
{
public:
  static InputParameters validParams();

  MooseVariableScalar(const InputParameters & parameters);
  virtual ~MooseVariableScalar();

  /**
   * Fill out the VariableValue arrays from the system solution vector
   * @param reinit_for_derivative_reordering A flag indicating whether we are reinitializing for the
   *        purpose of re-ordering derivative information for ADNodalBCs
   */
  void reinit(bool reinit_for_derivative_reordering = false);

  const VariableValue & sln() const { return _u; }

  /**
   * Return the solution with derivative information
   */
  const ADVariableValue & adSln() const;

  const VariableValue & slnOld() const;
  const VariableValue & slnOlder() const;
  const VariableValue & vectorTagSln(TagID tag) const;
  const VariableValue & matrixTagSln(TagID tag) const;

  const VariableValue & uDot() const;
  const VariableValue & uDotDot() const;
  const VariableValue & uDotOld() const;
  const VariableValue & uDotDotOld() const;
  const VariableValue & duDotDu() const;
  const VariableValue & duDotDotDu() const;

  /**
   * Return the first derivative of the solution with derivative information
   */
  const ADVariableValue & adUDot() const;

  /**
   * Set the nodal value for this variable (to keep everything up to date
   */
  void setValue(unsigned int i, Number value);

  /**
   * Set all of the values of this scalar variable to the same value
   */
  void setValues(Number value);

  void insert(NumericVector<Number> & soln);

  /**
   * The oldest solution state that is requested for this variable
   * (0 = current, 1 = old, 2 = older, etc).
   */
  unsigned int oldestSolutionStateRequested() const;

  void setActiveTags(const std::set<TagID> & vtags) override { _required_vector_tags = vtags; }

protected:
  /// The value of scalar variable
  VariableValue _u;
  /// The old value of scalar variable
  VariableValue _u_old;
  /// The older value of scalar variable
  VariableValue _u_older;
  /// Tagged vectors
  std::vector<VariableValue> _vector_tag_u;
  /// Only cache data when need it
  mutable std::vector<bool> _need_vector_tag_u;
  /// Tagged matrices
  std::vector<VariableValue> _matrix_tag_u;
  /// Only cache data when need it
  mutable std::vector<bool> _need_matrix_tag_u;

  VariableValue _u_dot;
  VariableValue _u_dotdot;
  VariableValue _u_dot_old;
  VariableValue _u_dotdot_old;
  VariableValue _du_dot_du;
  VariableValue _du_dotdot_du;

  mutable bool _need_u_dot;
  mutable bool _need_u_dotdot;
  mutable bool _need_u_dot_old;
  mutable bool _need_u_dotdot_old;
  mutable bool _need_du_dot_du;
  mutable bool _need_du_dotdot_du;

  /// Whether or not the old solution is needed
  mutable bool _need_u_old;
  /// Whether or not the older solution is needed
  mutable bool _need_u_older;

  /// Whether any AD calculations are needed
  mutable bool _need_ad;
  /// whether ad_u is needed
  mutable bool _need_ad_u;
  /// whether ad_u_dot is needed
  mutable bool _need_ad_u_dot;
  /// The scalar solution with derivative information
  ADVariableValue _ad_u;
  /// The first derivative of the scalar solution with derivative information
  ADVariableValue _ad_u_dot;

private:
  /**
   * Adds derivative information to the scalar variable value arrays
   * @param nodal_ordering Whether we are doing a nodal ordering of the derivative vector, e.g.
   *        whether the spacing between variable groups in the derivative vector is equal to the
   *        number of dofs per node or the number of dofs per elem
   */
  void computeAD(bool nodal_ordering);

  /// The set of vector tags we need to evaluate
  std::set<TagID> _required_vector_tags;
};
