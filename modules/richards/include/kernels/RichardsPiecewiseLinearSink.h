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

#ifndef RICHARDSPIECEWISELINEARSINK
#define RICHARDSPIECEWISELINEARSINK

#include "IntegratedBC.h"

#include "LinInt.h"

// Forward Declarations
class RichardsPiecewiseLinearSink;

template<>
InputParameters validParams<RichardsPiecewiseLinearSink>();

class RichardsPiecewiseLinearSink : public IntegratedBC
{
public:

  RichardsPiecewiseLinearSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  LinInt _sink_func;
  MaterialProperty<RealVectorValue> &_vel_SUPG;
  MaterialProperty<RealTensorValue> &_vel_prime_SUPG;
  MaterialProperty<Real> &_tau_SUPG;
  MaterialProperty<RealVectorValue> &_tau_prime_SUPG;
};

#endif //RichardsPiecewiseLinearSink
