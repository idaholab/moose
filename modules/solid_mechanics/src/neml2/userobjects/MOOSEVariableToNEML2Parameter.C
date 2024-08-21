//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEVariableToNEML2Parameter.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", MOOSEVariableToNEML2Parameter);

#ifndef NEML2_ENABLED
#define MOOSEMVariableToNEML2ParameterStub(name)                                                   \
  NEML2ObjectStubImplementationOpen(name, MOOSEToNEML2Parameter);                                  \
  NEML2ObjectStubVariable("moose_variable");                                                       \
  NEML2ObjectStubImplementationClose(name, MOOSEToNEML2Parameter)
MOOSEMVariableToNEML2ParameterStub(MOOSEVariableToNEML2Parameter)
#else

InputParameters
MOOSEVariableToNEML2Parameter::validParams()
{
  auto params = MOOSEToNEML2Parameter::validParams();
  params.addClassDescription(
      "Gather a MOOSE variable for insertion into the specified parameter of a "
      "NEML2 model.");

  params.addRequiredCoupledVar("moose_variable", "MOOSE variable to read from");
  return params;
}

MOOSEVariableToNEML2Parameter::MOOSEVariableToNEML2Parameter(const InputParameters & params)
  : MOOSEToNEML2Parameter(params), _moose_variable(coupledValue("moose_variable"))
{
}

torch::Tensor
MOOSEVariableToNEML2Parameter::convertQpMOOSEData() const
{
  return NEML2Utils::toNEML2<Real>(_moose_variable[_qp]);
}

#endif
