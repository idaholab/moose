/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTJACOBIANDAMPER_H
#define ELEMENTJACOBIANDAMPER_H

// Moose Includes
#include "GeneralDamper.h"
#include "MooseVariable.h"

// Forward Declarations
class ElementJacobianDamper;
class FEProblemBase;
class MooseMesh;
class DisplacedProblem;

template <>
InputParameters validParams<ElementJacobianDamper>();

/**
 * This class implements a damper that limits the change in the Jacobian of elements
 */
class ElementJacobianDamper : public GeneralDamper
{
public:
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
  QBase *& _qrule;

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

#endif // ELEMENTJACOBIANDAMPER_H
