//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTotalConcentrationAux.h"

registerKokkosAuxKernel("ChemicalReactionsApp", KokkosTotalConcentrationAux);

InputParameters
KokkosTotalConcentrationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addCoupledVar("primary_species", "Primary species free concentration");
  params.addParam<std::vector<Real>>(
      "sto_v",
      {},
      "The stoichiometric coefficient of primary species in secondary equilibrium species");
  params.addCoupledVar("v",
                       "Secondary equilibrium species in which the primary species is involved");
  params.addClassDescription("Total concentration of primary species (including stoichiometric "
                             "contribution to secondary equilibrium species)");
  return params;
}

KokkosTotalConcentrationAux::KokkosTotalConcentrationAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _primary_species(kokkosCoupledValue("primary_species")),
    _sto_v(getParam<std::vector<Real>>("sto_v")),
    _n(coupledComponents("v")),
    _secondary_species(kokkosCoupledValues("v"))
{
  // Check that the correct number of stoichiometric coefficients have been included
  if (_sto_v.size() != _n)
    mooseError("The number of stoichiometric coefficients and coupled species must be equal in ",
               _name);
}
