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
#ifndef COUPLEDEIGENKERNEL_H
#define COUPLEDEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class CoupledEigenKernel;

template <>
InputParameters validParams<CoupledEigenKernel>();

class CoupledEigenKernel : public EigenKernel
{
public:
  CoupledEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const VariableValue & _v;
};

#endif // COUPLEDEIGENKERNEL_H
