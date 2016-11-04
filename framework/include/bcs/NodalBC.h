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

#ifndef NODALBC_H
#define NODALBC_H

#include "BoundaryCondition.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

// Forward declarations
class NodalBC;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<NodalBC>();

/**
 * Base class for deriving any boundary condition that works at nodes
 */
class NodalBC : public BoundaryCondition,
                public RandomInterface,
                public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodalBC(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /// current node being processed
  const Node *& _current_node;

  /// Quadrature point index
  unsigned int _qp;
  /// Value of the unknown variable this BC is acting on
  const VariableValue & _u;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariable *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  virtual Real computeQpResidual() = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution for this NodalBC.  If not overriden,
   * returns 1.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif /* NODALBC_H */
