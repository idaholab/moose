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

// Forward Declarations
class InterfaceKernel;

template <>
InputParameters validParams<InterfaceKernel>();

/**
 * InterfaceKernel is responsible for interfacing physics across subdomains
 */

class InterfaceKernel : public DGKernel
{
public:
  InterfaceKernel(const InputParameters & parameters);

  /**
   * The neighbor variable number that this kernel operates on
   */
  const MooseVariable & neighborVariable() const;

  virtual void computeElemNeighResidual(Moose::DGResidualType type);
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);
  virtual void computeJacobian();
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);
  virtual void computeElementOffDiagJacobian(unsigned int jvar);
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  MooseVariable & _neighbor_var;

  const VariableValue & _neighbor_value;
  const VariableGradient & _grad_neighbor_value;
};

#endif
