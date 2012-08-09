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

#include "MTUserObject.h"

template<>
InputParameters validParams<MTUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addParam<Real>("scalar", 0, "A scalar value");
  params.addParam<std::vector<Real> >("vector", std::vector<Real>(), "A vector value");
  return params;
}


MTUserObject::MTUserObject(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _scalar(getParam<Real>("scalar")),
    _vector(getParam<std::vector<Real> >("vector")),
    _dyn_memory(NULL)
{
  // allocate some memory
  _dyn_memory = new Real[NUM];
}

MTUserObject::~MTUserObject()
{
}

void
MTUserObject::destroy()
{
  // release the Kraken (eeee... I mean memory)
  delete _dyn_memory;
}

Real
MTUserObject::doSomething() const
{
  // let's so something here, for example
  return -2.;
}


void
MTUserObject::load(std::ifstream & stream)
{
  stream.read((char *) & _scalar, sizeof(_scalar));
}

void
MTUserObject::store(std::ofstream & stream)
{
  stream.write((const char *) & _scalar, sizeof(_scalar));
}
