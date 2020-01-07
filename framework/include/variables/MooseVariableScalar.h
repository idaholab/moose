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

class MooseVariableScalar;

template <>
InputParameters validParams<MooseVariableScalar>();

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

  //
  VariableValue & sln() { return _u; }

  /**
   * Return the solution with derivative information
   */
  template <ComputeStage compute_stage>
  const ADVariableValue & adSln() const;

  VariableValue & slnOld() { return _u_old; }
  VariableValue & slnOlder() { return _u_older; }
  VariableValue & vectorTagSln(TagID tag)
  {
    _need_vector_tag_u[tag] = true;
    return _vector_tag_u[tag];
  }
  VariableValue & matrixTagSln(TagID tag)
  {
    _need_matrix_tag_u[tag] = true;
    return _matrix_tag_u[tag];
  }

  VariableValue & uDot()
  {
    if (_sys.solutionUDot())
    {
      _need_u_dot = true;
      return _u_dot;
    }
    else
      mooseError("MooseVariableScalar: Time derivative of solution (`u_dot`) is not stored. Please "
                 "set uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
  }

  VariableValue & uDotDot()
  {
    if (_sys.solutionUDotDot())
    {
      _need_u_dotdot = true;
      return _u_dotdot;
    }
    else
      mooseError("MooseVariableScalar: Second time derivative of solution (`u_dotdot`) is not "
                 "stored. Please set uDotDotRequested() to true in FEProblemBase before requesting "
                 "`u_dotdot`.");
  }

  VariableValue & uDotOld()
  {
    if (_sys.solutionUDotOld())
    {
      _need_u_dot_old = true;
      return _u_dot_old;
    }
    else
      mooseError("MooseVariableScalar: Old time derivative of solution (`u_dot_old`) is not "
                 "stored. Please set uDotOldRequested() to true in FEProblemBase before requesting "
                 "`u_dot_old`.");
  }

  VariableValue & uDotDotOld()
  {
    if (_sys.solutionUDotDotOld())
    {
      _need_u_dotdot_old = true;
      return _u_dotdot_old;
    }
    else
      mooseError("MooseVariableScalar: Old second time derivative of solution (`u_dotdot_old`) is "
                 "not stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
                 "requesting `u_dotdot_old`.");
  }

  VariableValue & duDotDu()
  {
    _need_du_dot_du = true;
    return _du_dot_du;
  }

  VariableValue & duDotDotDu()
  {
    _need_du_dotdot_du = true;
    return _du_dotdot_du;
  }

  /**
   * Set the nodal value for this variable (to keep everything up to date
   */
  void setValue(unsigned int i, Number value);

  /**
   * Set all of the values of this scalar variable to the same value
   */
  void setValues(Number value);

  void insert(NumericVector<Number> & soln);

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
  std::vector<bool> _need_vector_tag_u;
  /// Tagged matrices
  std::vector<VariableValue> _matrix_tag_u;
  /// Only cache data when need it
  std::vector<bool> _need_matrix_tag_u;

  VariableValue _u_dot;
  VariableValue _u_dotdot;
  VariableValue _u_dot_old;
  VariableValue _u_dotdot_old;
  VariableValue _du_dot_du;
  VariableValue _du_dotdot_du;

  bool _need_u_dot;
  bool _need_u_dotdot;
  bool _need_u_dot_old;
  bool _need_u_dotdot_old;
  bool _need_du_dot_du;
  bool _need_du_dotdot_du;

  /// Whether any dual number calculations are needed
  mutable bool _need_dual;
  /// whether dual_u is needed
  mutable bool _need_dual_u;
  /// The scalar solution with derivative information
  DualVariableValue _dual_u;

private:
  /**
   * Adds derivative information to the scalar variable value arrays
   * @param nodal_ordering Whether we are doing a nodal ordering of the derivative vector, e.g.
   *        whether the spacing between variable groups in the derivative vector is equal to the
   *        number of dofs per node or the number of dofs per elem
   */
  void computeAD(bool nodal_ordering);
};

template <>
const VariableValue & MooseVariableScalar::adSln<ComputeStage::RESIDUAL>() const;

template <>
const DualVariableValue & MooseVariableScalar::adSln<ComputeStage::JACOBIAN>() const;
