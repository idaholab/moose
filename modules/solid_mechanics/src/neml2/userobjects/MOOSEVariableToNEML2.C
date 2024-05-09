//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEVariableToNEML2.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", MOOSEVariableToNEML2);

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementationOpen(MOOSEVariableToNEML2, MOOSEToNEML2);
NEML2ObjectStubParam(std::vector<VariableName>, "moose_variable");
NEML2ObjectStubParam(std::string, "neml2_variable");
NEML2ObjectStubImplementationClose(MOOSEVariableToNEML2, MOOSEToNEML2);
#else

InputParameters
MOOSEVariableToNEML2::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params.addClassDescription("Gather a MOOSE variable for insertion into the specified input of a "
                             "NEML2 model using the .");

  params.addRequiredCoupledVar("moose_variable", "MOOSE variable to read from");
  return params;
}

MOOSEVariableToNEML2::MOOSEVariableToNEML2(const InputParameters & params)
  : MOOSEToNEML2(params), _moose_variable(coupledValue("moose_variable"))
{
}

void
MOOSEVariableToNEML2::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    _buffer.push_back(NEML2Utils::toNEML2<Real>(_moose_variable[qp]));
}

#endif
