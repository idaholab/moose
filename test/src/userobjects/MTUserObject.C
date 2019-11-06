//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTUserObject.h"

#include <fstream>

registerMooseObject("MooseTestApp", MTUserObject);

InputParameters
MTUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<Real>("scalar", 0, "A scalar value");
  params.addParam<std::vector<Real>>("vector", std::vector<Real>(), "A vector value");
  return params;
}

MTUserObject::MTUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    _scalar(getParam<Real>("scalar")),
    _vector(getParam<std::vector<Real>>("vector")),
    _dyn_memory(NULL)
{
  // allocate some memory
  _dyn_memory = new Real[NUM];
}

MTUserObject::~MTUserObject() { delete[] _dyn_memory; }

Real
MTUserObject::doSomething() const
{
  // let's so something here, for example
  return -2.;
}

void
MTUserObject::load(std::ifstream & stream)
{
  stream.read((char *)&_scalar, sizeof(_scalar));
}

void
MTUserObject::store(std::ofstream & stream)
{
  stream.write((const char *)&_scalar, sizeof(_scalar));
}
