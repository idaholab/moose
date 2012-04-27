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

#include "MTUserData.h"

template<>
InputParameters validParams<MTUserData>()
{
  InputParameters params = validParams<UserObject>();
  params.addParam<Real>("scalar", 0, "A scalar value");
  params.addParam<std::vector<Real> >("vector", std::vector<Real>(), "A vector value");
  return params;
}


MTUserData::MTUserData(const std::string & name, InputParameters params) :
    UserObject(name, params),
    _scalar(getParam<Real>("scalar")),
    _vector(getParam<std::vector<Real> >("vector")),
    _dyn_memory(NULL)
{
  // allocate some memory
  _dyn_memory = new Real[NUM];
}

MTUserData::~MTUserData()
{
}

void
MTUserData::destroy()
{
  // release the Kraken (eeee... I mean memory)
  delete _dyn_memory;
}

Real
MTUserData::doSomething() const
{
  // let so something here, for example
  return -2.;
}
