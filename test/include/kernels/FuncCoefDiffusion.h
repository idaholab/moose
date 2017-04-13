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
#ifndef FUNCCOEFDIFFUSION_H
#define FUNCCOEFDIFFUSION_H

// MOOSE includes
#include "Kernel.h"
#include "Function.h"

// Forward Declarations
class FuncCoefDiffusion;

template <>
InputParameters validParams<FuncCoefDiffusion>();

/**
 * A kernel for testing the MooseParsedFunctionInterface
 */
class FuncCoefDiffusion : public Kernel
{
public:
  FuncCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Function & _function;
};

#endif // FUNCCOEFDIFFUSION_H
