/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#include <iostream>
#include <string>
#include <cmath>
#include "ThermochimicaAux.h"
#include "Thermochimica.h"

#include "Material.h"

#ifdef THERMOCHIMICA_ENABLED
registerMooseObject("BisonApp", ThermochimicaAux);
#endif

InputParameters
ThermochimicaAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("elements", "Amounts of elements");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addRequiredParam<UserObjectName>("thermo_nodal_data_uo", "Name of the user object");
  params.addCoupledVar("element_potentials", "Chemical potentials of elements");

  params.addClassDescription("Thermodynamics calculations for oxygen transport model.");

  return params;
}

ThermochimicaAux::ThermochimicaAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _phases_coupled(isCoupled("output_phases")),
    _species_coupled(isCoupled("output_species")),
    _output_element_potential(isCoupled("element_potentials")),
    _thermo_nodal_data_uo(&getUserObject<ThermochimicaNodalData>("thermo_nodal_data_uo"))
{
#ifndef THERMOCHIMICA_ENABLED
  mooseError("Thermochimica disabled");
#endif
  if (_phases_coupled)
  {
    _n_phases = coupledComponents("output_phases");
    _ph.resize(_n_phases);
    _ph_name.resize(_n_phases);
    for (unsigned int i = 0; i < _n_phases; i++)
    {
      _ph[i] = &writableCoupledValue("output_phases", i);
      MooseVariable * mv = getVar("output_phases", i);
      _ph_name[i] = mv->name();
    }
  }
  else
  {
    _n_phases = 0;
  }
  if (_species_coupled)
  {
    _n_species = coupledComponents("output_species");
    _sp.resize(_n_species);
    _sp_phase_name.resize(_n_species);
    _sp_species_name.resize(_n_species);
    for (unsigned int i = 0; i < _n_species; i++)
    {
      _sp[i] = &writableCoupledValue("output_species", i);
      MooseVariable * mv = getVar("output_species", i);
      std::string species_var_name = mv->name();
      int semicolon = species_var_name.find(";");
      _sp_phase_name[i] = species_var_name.substr(0, semicolon);
      _sp_species_name[i] = species_var_name.substr(semicolon + 1);
    }
  }
  else
  {
    _n_species = 0;
  }

  if (_output_element_potential)
  {
    unsigned int n_elems = coupledComponents("element_potentials");
    _el_pot.resize(n_elems);
    for (unsigned int i = 0; i < n_elems; i++)
    {
      _el_pot[i] = &writableCoupledValue("element_potentials", i);
    }
  }
}

Real
ThermochimicaAux::computeValue()
{
#ifdef THERMOCHIMICA_ENABLED

  // Save reqested output phase and species data
  if (_phases_coupled)
  {
    std::vector<double> dMolesPhase = _thermo_nodal_data_uo->getMolesPhase(_current_node->id());
    std::vector<int> iPhaseIndices = _thermo_nodal_data_uo->getPhaseIndices(_current_node->id());
    for (unsigned int i = 0; i < _n_phases; i++)
    {
      if (iPhaseIndices[i] < 0)
      {
        (*_ph[i])[_qp] = 0;
      }
      else
      {
        (*_ph[i])[_qp] = dMolesPhase[iPhaseIndices[i]];
      }
    }
  }
  if (_species_coupled)
  {
    std::vector<double> dSpeciesFractions =
        _thermo_nodal_data_uo->getSpeciesFractions(_current_node->id());
    for (unsigned int i = 0; i < _n_species; i++)
    {
      (*_sp[i])[_qp] = dSpeciesFractions[i];
    }
  }

  if (_output_element_potential)
  {
    std::vector<double> dElementPotentials =
        _thermo_nodal_data_uo->getElementPotential(_current_node->id());
    for (unsigned int i = 0; i < _el_pot.size(); i++)
    {
      (*_el_pot[i])[_qp] = dElementPotentials[i];
    }
  }

#endif
  return 0;
}
