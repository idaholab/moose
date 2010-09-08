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

#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

#include "ImplicitEuler.h"

// Forward Declarations
class ExampleImplicitEuler;

template<>
InputParameters validParams<ExampleImplicitEuler>();

class ExampleImplicitEuler : public ImplicitEuler
{
public:

  ExampleImplicitEuler(const std::string & name,
                       MooseSystem &sys,
                       InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _time_coefficient;
};
#endif //EXAMPLEIMPLICITEULER
