//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalConcentrationAux.h"

registerMooseObject("ChemicalReactionsApp", TotalConcentrationAux);

InputParameters
TotalConcentrationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addCoupledVar("primary_species", "Primary species free concentration");
  params.addParam<std::vector<Real>>(
      "sto_v",
      "The stoichiometric coefficient of primary species in secondary equilibrium species");
  params.addCoupledVar("v",
                       "Secondary equilibrium species in which the primary species is involved");
  params.addClassDescription("Total concentration of primary species (including stoichiometric "
                             "contribution to secondary equilibrium species)");
  return params;
}

TotalConcentrationAux::TotalConcentrationAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _primary_species(coupledValue("primary_species")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _secondary_species(coupledValues("v"))
{
  // Check that the correct number of stoichiometric coefficients have been included
  if (_sto_v.size() != coupledComponents("v"))
    mooseError("The number of stoichiometric coefficients and coupled species must be equal in ",
               _name);
}

Real
TotalConcentrationAux::computeValue()
{
  Real total_concentration = _primary_species[_qp];

  for (unsigned int i = 0; i < _secondary_species.size(); ++i)
    total_concentration += _sto_v[i] * (*_secondary_species[i])[_qp];

  return total_concentration;
}
