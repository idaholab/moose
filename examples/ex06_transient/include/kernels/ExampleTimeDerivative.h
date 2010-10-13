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

#ifndef EXAMPLETIMEDERIVATIVE
#define EXAMPLETIMEDERIVATIVE

#include "TimeDerivative.h"

// Forward Declarations
class ExampleTimeDerivative;

template<>
InputParameters validParams<ExampleTimeDerivative>();

class ExampleTimeDerivative : public TimeDerivative
{
public:

  ExampleTimeDerivative(const std::string & name,
                        MooseSystem &sys,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _time_coefficient;
};

#endif //EXAMPLETIMEDERIVATIVE
