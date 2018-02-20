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
#ifndef LINEARVECTORPOISSON_H
#define LINEARVECTORPOISSON_H

#include "VectorKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class LinearVectorPoisson;

template <>
InputParameters validParams<LinearVectorPoisson>();

class LinearVectorPoisson : public VectorKernel
{
public:
  LinearVectorPoisson(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Function & _x_sln;
  Function & _y_sln;
  const Real _eps;
};

#endif // LINEARVECTORPOISSON_H
