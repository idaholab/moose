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

#ifndef RICHARDSHALFGAUSSIANSINK
#define RICHARDSHALFGAUSSIANSINK

#include "IntegratedBC.h"

#include "LinInt.h"

// Forward Declarations
class RichardsHalfGaussianSink;

template<>
InputParameters validParams<RichardsHalfGaussianSink>();

class RichardsHalfGaussianSink : public IntegratedBC
{
public:

  RichardsHalfGaussianSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _maximum;
  Real _sd;
  Real _centre;
  MaterialProperty<RealVectorValue> &_vel_SUPG;
  MaterialProperty<RealTensorValue> &_vel_prime_SUPG;
  MaterialProperty<Real> &_tau_SUPG;
  MaterialProperty<RealVectorValue> &_tau_prime_SUPG;

};

#endif //RICHARDSHALFGAUSSIANSINK
