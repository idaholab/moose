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

#include "MultipleUpdateElemAux.h"

template<>
InputParameters validParams<MultipleUpdateElemAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("vars", "unknown (nl-variable)");

  return params;
}

MultipleUpdateElemAux::MultipleUpdateElemAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _n_vars(coupledComponents("vars"))
{
  for (unsigned int i=0; i<_n_vars; i++)
  {
    _vars.push_back(getVar("vars", i));
    if (_vars[i]->isNodal()) mooseError("variables have to be elemental");
  }
  if (isNodal()) mooseError("variable have to be elemental");
}

MultipleUpdateElemAux::~MultipleUpdateElemAux()
{
}

void
MultipleUpdateElemAux::compute()
{
  std::vector<Real> values(_n_vars);

  computeVarValues(values);

  for (unsigned int i=0; i<_n_vars; i++)
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
  for (unsigned int i=0; i<values.size(); i++)
    values[i] = 100+i;
}
