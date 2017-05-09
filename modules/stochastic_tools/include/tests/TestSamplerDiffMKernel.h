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
#ifndef TESTSAMPLERDIFFMKERNEL_H
#define TESTSAMPLERDIFFMKERNEL_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class TestSamplerDiffMKernel;

template <>
InputParameters validParams<TestSamplerDiffMKernel>();

class TestSamplerDiffMKernel : public Kernel
{
public:
  TestSamplerDiffMKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _diff;
  Real _offset;
};
#endif // TESTSAMPLERDIFFMKERNEL_H
