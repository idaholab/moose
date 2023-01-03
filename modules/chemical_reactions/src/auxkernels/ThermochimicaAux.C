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
    _ph_name(_n_phases),
    _n_species(coupledComponents("output_species")),
    _sp(_n_species),
    _sp_phase_name(_n_species),
    _sp_species_name(_n_species),
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
  {
    _ph[i] = &writableCoupledValue("output_phases", i);
    _ph_name[i] = getVar("output_phases", i)->name();
  }

  for (const auto i : make_range(_n_species))
  {
    _sp[i] = &writableCoupledValue("output_species", i);
    auto species_var_name = getVar("output_species", i)->name();
    int semicolon = species_var_name.find(";");
    _sp_phase_name[i] = species_var_name.substr(0, semicolon);
    _sp_species_name[i] = species_var_name.substr(semicolon + 1);
  }

  for (const auto i : make_range(_n_elements))
    _el_pot[i] = &writableCoupledValue("element_potentials", i);
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
      (*_ph[i])[_qp] = 0;
    else
    {
      (*_ph[i])[_qp] = data._moles_phase[data._phase_indices[i]];
      n_active_phases += 1.0;
    }

  // Save requested species data into coupled aux variables
  for (unsigned int i = 0; i < _n_species; i++)
    (*_sp[i])[_qp] = data._species_fractions[i];

  // Save requested element potentials into coupled aux variables
  mooseAssert(_el_pot.size() == data._element_potential.size(), "Inconsistent sizes.");
  for (const auto i : make_range(_n_elements))
    (*_el_pot[i])[_qp] = data._element_potential[i];
#endif

  return n_active_phases;
}
