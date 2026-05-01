//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2.h"

InputParameters
MOOSEToNEML2::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<std::string>("to_neml2",
                                       "Name of the NEML2 variable or parameter to write to");
  return params;
}

#ifndef NEML2_ENABLED

MOOSEToNEML2::MOOSEToNEML2(const InputParameters & /*params*/) {}

#else

MOOSEToNEML2::MOOSEToNEML2(const InputParameters & params)
  : _neml2_name(params.get<std::string>("to_neml2"))
{
  NEML2Utils::assertNEML2Enabled();
}

void
MOOSEToNEML2::insertInto(std::map<std::string, neml2::Tensor> & map) const
{
  map[_neml2_name] = gatheredData();
}
#endif
