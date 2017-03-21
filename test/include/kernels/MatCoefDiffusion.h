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
#ifndef MATCOEFDIFFUSION_H
#define MATCOEFDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class MatCoefDiffusion;

template <>
InputParameters validParams<MatCoefDiffusion>();

/**
 * A test class for checking the operation for BlockRestrictable::hasMaterialProperty
 */
class MatCoefDiffusion : public Kernel
{
public:
  MatCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  std::string _prop_name;
  const MaterialProperty<Real> * _coef;
};

#endif // MATCOEFDIFFUSION_H
