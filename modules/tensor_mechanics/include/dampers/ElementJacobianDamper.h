//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
};
