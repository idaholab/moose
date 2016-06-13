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

#ifndef NONLOCALKERNEL_H
#define NONLOCALKERNEL_H

#include "Kernel.h"

class NonlocalKernel;

template<>
InputParameters validParams<NonlocalKernel>();

class NonlocalKernel :
  public Kernel
{
public:
  NonlocalKernel(const InputParameters & parameters);

  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /// Compute this Kernel's contribution to the Jacobian corresponding to nolocal dof at the current quadrature point
  virtual Real computeQpNonlocalJacobian(dof_id_type dof_index);
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index);

  DenseMatrix<Number> _nonlocal_ke;
  unsigned int _k;
};

#endif /* NONLOCALKERNEL_H */
