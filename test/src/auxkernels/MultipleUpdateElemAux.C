//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultipleUpdateElemAux.h"

registerMooseObject("MooseTestApp", MultipleUpdateElemAux);

InputParameters
MultipleUpdateElemAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("vars",
                               "Coupled variables that will be written to by the test object.");
  return params;
}

MultipleUpdateElemAux::MultipleUpdateElemAux(const InputParameters & parameters)
  : AuxKernel(parameters), _n_vars(coupledComponents("vars"))
{
  for (unsigned int i = 0; i < _n_vars; i++)
    _vars.push_back(&writableVariable("vars", i));
}

MultipleUpdateElemAux::~MultipleUpdateElemAux() {}

void
MultipleUpdateElemAux::compute()
{
  std::vector<Real> values(_n_vars);

  computeVarValues(values);

  for (unsigned int i = 0; i < _n_vars; i++)
    _vars[i]->setNodalValue(values[i]);

  _var.setNodalValue(100.0);
}

Real
MultipleUpdateElemAux::computeValue()
{
  // never executed
  return 100.0;
}

void
MultipleUpdateElemAux::computeVarValues(std::vector<Real> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    values[i] = 100 + i;
}
