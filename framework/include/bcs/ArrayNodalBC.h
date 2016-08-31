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

#ifndef ARRAYNODALBC_H
#define ARRAYNODALBC_H

#include "NodalBCBase.h"

// Forward declarations
class ArrayNodalBC;

// libMesh forward declarations
namespace libMesh
{
template <typename T> class NumericVector;
}

template<>
InputParameters validParams<ArrayNodalBC>();

/**
 * Base class for deriving any boundary condition that works at nodes
 */
class ArrayNodalBC : public NodalBCBase
{
public:
  ArrayNodalBC(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /// The MooseVariable this BC is acting on
  ArrayMooseVariable & _array_var;

  /// current node being processed
  const Node * & _current_node;

  /// The residual values to be set in computeQpResidual()
  Eigen::Map<Eigen::VectorXd> _residual;

  /// Quadrature point index
  unsigned int _qp;

  /// Value of the unknown variable this BC is acting on
  const ArrayVariableValue & _u;

  virtual void computeQpResidual() = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution for this ArrayNodalBC.  If not overriden,
   * returns 1.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif /* ARRAYNODALBC_H */
