//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "GeneralDamper.h"
#include "MooseVariable.h"

// Forward Declarations
class FEProblemBase;
class MooseMesh;
class DisplacedProblem;

/**
 * This class implements a damper that limits the change in the Jacobian of elements
 */
class ElementJacobianDamper : public GeneralDamper
{
public:
  static InputParameters validParams();

  ElementJacobianDamper(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * Computes this Damper's damping
   */
  virtual Real computeDamping(const NumericVector<Number> & /* solution */,
                              const NumericVector<Number> & update) override;

  /**
   * Probe a damped update on the displaced mesh and report the maximum relative
   * change in JxW. Returns false if the trial update produces a degenerate map.
   */
  bool probeDamping(const NumericVector<Number> & update,
                    Real damping,
                    Real & max_difference,
                    std::string & error_message);

protected:
  /// Thread ID
  THREAD_ID _tid;
  Assembly & _assembly;

  /// Quadrature rule
  const QBase * const & _qrule;

  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;

  /// The FE problem
  FEProblemBase & _fe_problem;

  /// The displaced problem
  MooseSharedPointer<DisplacedProblem> _displaced_problem;

  /// The displaced mesh
  MooseMesh * _mesh;

  /// The displacement variables
  std::vector<MooseVariable *> _disp_var;

  /// The number of displacement variables
  unsigned int _ndisp;

  /// The current Newton increment in the displacement variables
  std::vector<VariableValue> _disp_incr;

  /// Maximum allowed relative increment in Jacobian
  const Real _max_jacobian_diff;

  /// Whether to backtrack the trial update when probing would otherwise create a degenerate map
  const bool _use_backtracking;

  /// Multiplicative cutback applied during backtracking
  const Real _backtrack_factor;

  /// Maximum number of backtracking attempts
  const unsigned int _max_backtrack_steps;
};
