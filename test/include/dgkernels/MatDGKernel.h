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

#ifndef MATDGKERNEL_H
#define MATDGKERNEL_H

#include "DGKernel.h"

// Forward Declarations
class MatDGKernel;

template <>
InputParameters validParams<MatDGKernel>();

/**
 * This is for testing errors, it does nothing.
 */
class MatDGKernel : public DGKernel
{
public:
  MatDGKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType) override { return 0.0; }
  virtual Real computeQpJacobian(Moose::DGJacobianType) override { return 0.0; }
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType, unsigned int) override
  {
    return 0.0;
  }

  const MaterialProperty<Real> & _value;
};

#endif
