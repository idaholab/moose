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
#ifndef CONVECTIONPRECOMPUTE_H
#define CONVECTIONPRECOMPUTE_H

#include "KernelValue.h"

// Forward Declarations
class ConvectionPrecompute;

template <>
InputParameters validParams<ConvectionPrecompute>();

class ConvectionPrecompute : public KernelValue
{
public:
  ConvectionPrecompute(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();

private:
  RealVectorValue _velocity;
};

#endif // CONVECTIONPRECOMPUTE_H
