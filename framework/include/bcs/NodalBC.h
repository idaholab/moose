//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  void setBCOnEigen(bool iseigen) { _is_eigen = iseigen; }

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

  /// Indicate whether or not the boundary condition is applied to the right
  /// hand side of eigenvalue problems
  bool _is_eigen;

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
