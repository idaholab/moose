//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultipleUpdateElemAux.h"

template <>
InputParameters
validParams<MultipleUpdateElemAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("vars", "unknown (nl-variable)");

  return params;
}

MultipleUpdateElemAux::MultipleUpdateElemAux(const InputParameters & parameters)
  : AuxKernel(parameters), _n_vars(coupledComponents("vars"))
{
  for (unsigned int i = 0; i < _n_vars; i++)
  {
    _vars.push_back(getVar("vars", i));
    if (_vars[i]->isNodal())
      mooseError("variables have to be elemental");
  }
  if (isNodal())
    mooseError("variable have to be elemental");
}

MultipleUpdateElemAux::~MultipleUpdateElemAux() {}

void
MultipleUpdateElemAux::compute()
{
  std::vector<Real> values(_n_vars);

  computeVarValues(values);

  for (unsigned int i = 0; i < _n_vars; i++)
    _vars[i]->setNodalValue(values[i]);

  _var.setNodalValue(0.0);
}

Real
MultipleUpdateElemAux::computeValue()
{
  return 0.0;
}

void
MultipleUpdateElemAux::computeVarValues(std::vector<Real> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    values[i] = 100 + i;
}
