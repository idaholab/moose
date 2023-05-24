//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaAux.h"
#include "ThermochimicaUtils.h"
#include "libmesh/int_range.h"
#include "libmesh/fe.h"

#include <iostream>
#include <string>
#include <cmath>

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#endif

registerMooseObject("ChemicalReactionsApp", ThermochimicaAux);

InputParameters
ThermochimicaAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("elements", "Amounts of elements");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addRequiredParam<UserObjectName>("thermo_nodal_data_uo", "Name of the user object");
  params.addCoupledVar("element_potentials", "Chemical potentials of elements");
  params.addCoupledVar("output_vapor_pressures", "Vapour pressures of species to be output");

  ThermochimicaUtils::addClassDescription(
      params,
      "Extracts phase and species amounts, and element chemical potentials "
      "from a Thermochimica user object.");

  return params;
}

ThermochimicaAux::ThermochimicaAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _n_phases(coupledComponents("output_phases")),
    _ph(_n_phases),
    _n_species(coupledComponents("output_species")),
    _sp(_n_species),
    _n_vapor_species(coupledComponents("output_vapor_pressures")),
    _vapor_pressures(_n_vapor_species),
    _n_elements(coupledComponents("element_potentials")),
    _el_pot(_n_elements)
#ifdef THERMOCHIMICA_ENABLED
    ,
    _thermo_nodal_data_uo(getUserObject<ThermochimicaNodalData>("thermo_nodal_data_uo"))
#endif
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

  if (!isNodal())
    paramError("variable", "A nodal variable must be supplied.");
  for (const auto v : _coupled_moose_vars)
    if (v->feType() != FEType(FIRST, LAGRANGE))
      mooseError("All variables coupled in ThermochimicaAux must be of first order Lagrange type.");

  for (const auto i : make_range(_n_phases))
    _ph[i] = &writableVariable("output_phases", i);

  for (const auto i : make_range(_n_species))
    _sp[i] = &writableVariable("output_species", i);

  for (const auto i : make_range(_n_vapor_species))
    _vapor_pressures[i] = &writableVariable("output_vapor_pressures", i);

  for (const auto i : make_range(_n_elements))
    _el_pot[i] = &writableVariable("element_potentials", i);
}

Real
ThermochimicaAux::computeValue()
{
  // return the number of active phases
  Real n_active_phases = 0.0;

#ifdef THERMOCHIMICA_ENABLED
  const auto & data = _thermo_nodal_data_uo.getNodalData(_current_node->id());

  // Save requested phase data into coupled aux variables
  for (const auto i : make_range(_n_phases))
    if (data._phase_indices[i] < 0)
      _ph[i]->setNodalValue(0, _qp);
    else
    {
      _ph[i]->setNodalValue(data._moles_phase[data._phase_indices[i]], _qp);
      n_active_phases += 1.0;
    }

  // Save requested species data into coupled aux variables
  for (unsigned int i = 0; i < _n_species; i++)
    _sp[i]->setNodalValue(data._species_fractions[i], _qp);

  // Save requested vapor pressures into coupled aux variables
  mooseAssert(_vapor_pressures.size() == data._vapor_pressures.size(),
              "Output vapor pressures: Inconsistent sizes.");
  for (const auto i : make_range(_n_vapor_species))
    _vapor_pressures[i]->setNodalValue(data._vapor_pressures[i], _qp);

  // Save requested element potentials into coupled aux variables
  mooseAssert(_el_pot.size() == data._element_potential_for_output.size(),
              "Output element potentials: Inconsistent sizes.");
  for (const auto i : make_range(_n_elements))
    _el_pot[i]->setNodalValue(data._element_potential_for_output[i], _qp);
#endif

  return n_active_phases;
}
