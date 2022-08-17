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
#include "RankTwoTensorForward.h"
#include "Coupleable.h"
#include "BlockRestrictable.h"

// Forward Declarations
class FEProblemBase;
class MooseMesh;

/**
 * This class implements a damper that limits the change in the Jacobian of elements without relying
 * on having the displaced mesh
 */
class ReferenceElementJacobianDamper : public GeneralDamper,
                                       public Coupleable,
                                       public BlockRestrictable
{
public:
  static InputParameters validParams();

  ReferenceElementJacobianDamper(const InputParameters & parameters);

  virtual void initialSetup() override {}

  virtual Real computeDamping(const NumericVector<Number> & solution,
                              const NumericVector<Number> & update) override;

protected:
  /// Maximum allowed relative increment in Jacobian
  const Real _max_jacobian_diff;

  /// Thread ID
  THREAD_ID _tid;

  /// The undisplaced mesh
  MooseMesh & _mesh;

  /// The undisplaced assembly
  Assembly & _assembly;

  /// Quadrature rule
  const QBase * const & _qrule;

  /// Number of displacement variables
  const unsigned int _ndisp;

  /// The displacement variable numbers
  std::vector<unsigned int> _disp_num;

  /// shape function gradients
  std::vector<const VariablePhiGradient *> _grad_phi;

private:
  /// Fill the displacement gradients
  void computeGradDisp(const Elem * elem,
                       const NumericVector<Number> & solution,
                       const NumericVector<Number> & update);

  /// The current displacement gradients
  std::vector<std::vector<RealVectorValue>> _grad_disp;

  /// The displacement gradients after this update
  std::vector<std::vector<RealVectorValue>> _grad_disp_update;
};
