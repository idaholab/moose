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

#ifndef INTERFACEKERNEL_H
#define INTERFACEKERNEL_H

#include "DGKernel.h"

//Forward Declarations
class InterfaceKernel;

template<>
InputParameters validParams<InterfaceKernel>();

/**
 * InterfaceKernel is responsible for interfacing physics across subdomains
 */

class InterfaceKernel : public DGKernel
{
public:
  InterfaceKernel(const InputParameters & parameters);

  virtual void computeElemNeighResidual(Moose::DGResidualType type);
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);
  virtual void computeJacobian(unsigned int jvar);
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);
  virtual void computeOffDiagJacobian(unsigned int jvar);

  // Don't let our computeJacobian(unsigned) hide DGKernel::computeJacobian(void)
  using DGKernel::computeJacobian;

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  MooseVariable & _neighbor_var;

  const VariableValue & _neighbor_value;
  const VariableGradient & _grad_neighbor_value;
};

#endif
