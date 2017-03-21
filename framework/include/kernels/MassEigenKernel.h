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

#ifndef MASSEIGENKERNEL_H
#define MASSEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class MassEigenKernel;

template <>
InputParameters validParams<MassEigenKernel>();

class MassEigenKernel : public EigenKernel
{
public:
  MassEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};

#endif // MASSEIGENKERNEL_H
