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

#include "ThermochimicaNodalData.h"
#include "Thermochimica.h"

registerMooseObject("BisonApp", ThermochimicaNodalData);

InputParameters
ThermochimicaNodalData::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addRequiredCoupledVar("elements", "Amounts of elements");
  params.addParam<Real>("pressure", 1.0, "Pressure");
  params.addRequiredCoupledVar("temp", "Coupled temperature");
  params.addParam<int>("reinit_requested", 1, "Should Thermochimica use re-initialization?");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addCoupledVar("element_potentials", "Chemical potentials of elements");

  params.addClassDescription("Provides access to Thermochimica-calculated data at nodes");

  return params;
}

ThermochimicaNodalData::ThermochimicaNodalData(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _pressure(getParam<Real>("pressure")),
    _temp(coupledValue("temp")),
    _reinit_requested(getParam<int>("reinit_requested")),
    _phases_coupled(isCoupled("output_phases")),
    _species_coupled(isCoupled("output_species")),
    _output_element_potential(isCoupled("element_potentials"))
{
  unsigned int n_elems = coupledComponents("elements");
  _el.resize(n_elems);
  _el_name.resize(n_elems);
  for (unsigned int i = 0; i < n_elems; i++)
  {
    _el[i] = &coupledValue("elements", i);
    MooseVariable * mv = getVar("elements", i);
    _el_name[i] = mv->name();
  }

  if (_phases_coupled)
  {
    _n_phases = coupledComponents("output_phases");
    _ph_name.resize(_n_phases);
    for (unsigned int i = 0; i < _n_phases; i++)
    {
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
    _sp_phase_name.resize(_n_species);
    _sp_species_name.resize(_n_species);
    for (unsigned int i = 0; i < _n_species; i++)
    {
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
    _element_potentials.resize(n_elems);
    for (unsigned int i = 0; i < n_elems; i++)
    {
      MooseVariable * mv = getVar("element_potentials", i);
      std::string element_var_name = mv->name();
      int semicolon = element_var_name.find(";");
      _element_potentials[i] = element_var_name.substr(semicolon + 1);
    }
  }
}

void
ThermochimicaNodalData::initialize()
{
}

void
ThermochimicaNodalData::execute()
{
#ifdef THERMOCHIMICA_ENABLED

  Real Temperature = _temp[_qp];
  Real Pressure = _pressure;

  Real dMol;
  int iElement;
  int idbg = 0;

  // Set temperature and pressure for thermochemistry solver
  FORTRAN_CALL(Thermochimica::settemperaturepressure)(&Temperature, &Pressure);

  iElement = 0;
  dMol = 0.0;
  FORTRAN_CALL(Thermochimica::setelementmass)(&iElement, &dMol); // Reset all element masses to 0

  // Set element masses
  for (unsigned int i = 0; i < _el.size(); i++)
  {
    iElement = Thermochimica::atomicNumber(_el_name[i]);
    dMol = (*_el[i])[_qp];
    FORTRAN_CALL(Thermochimica::setelementmass)(&iElement, &dMol);
  }

  // Optionally ask for a re-initialization (if reinit_requested > 0)
  reinitDataMooseToTc();
  // Calculate thermochemical equilibrium
  FORTRAN_CALL(Thermochimica::thermochimica)();
  // if (_current_node->id() == 50) {
  //   int iPrint = 2;
  //   FORTRAN_CALL(Thermochimica::setprintresultsmode)(&iPrint);
  //   FORTRAN_CALL(Thermochimica::printresults)();
  // }
  // Check for error status
  FORTRAN_CALL(Thermochimica::checkinfothermo)(&idbg);
  if (idbg != 0)
  {
    Moose::out << "thermochimica error " << idbg << "\n";
    FORTRAN_CALL(Thermochimica::printstate)();
    // Moose::out << Temperature << "\n";
    // for (unsigned int i = 0; i < _el.size(); i++)
    // {
    //   iElement = Thermochimica::atomicNumber(_el_name[i]);
    //   dMol = (*_el[i])[_qp];
    //   Moose::out << iElement << " " << dMol << "\n";
    // }
  }
  else
  {
    // Get requested phase indices if phase concentration output was requested
    // i.e. if output_phases is coupled
    if (_phases_coupled)
    {
      _phase_indices[_current_node->id()].resize(_n_phases);
      for (unsigned int i = 0; i < _n_phases; i++)
      {
        char cPhaseName[20];
        int lcPhaseName = sizeof cPhaseName;
        Thermochimica::ConvertToFortran(cPhaseName, lcPhaseName, _ph_name[i].c_str());
        int index = 0;
        FORTRAN_CALL(Thermochimica::getphaseindex)(cPhaseName, &lcPhaseName, &index, &idbg);
        // Convert from 1-based (fortran) to 0-based (c++) indexing
        _phase_indices[_current_node->id()][i] = index - 1;
      }
    }

    // Save data for future reinits
    reinitDataMooseFromTc();

    _species_fractions[_current_node->id()].resize(_n_species);
    for (unsigned int i = 0; i < _n_species; i++)
    {
      char cPhaseName[25];
      int lcPhaseName = sizeof cPhaseName;
      Thermochimica::ConvertToFortran(cPhaseName, lcPhaseName, _sp_phase_name[i].c_str());
      char cSpeciesName[25];
      int lcSpeciesName = sizeof cSpeciesName;
      Thermochimica::ConvertToFortran(cSpeciesName, lcSpeciesName, _sp_species_name[i].c_str());
      FORTRAN_CALL(Thermochimica::getoutputmolspeciesphase)
      (cPhaseName,
       &lcPhaseName,
       cSpeciesName,
       &lcSpeciesName,
       &_species_fractions[_current_node->id()][i],
       &idbg);
      if (idbg == 1)
        _species_fractions[_current_node->id()][i] = 0;
      else if (idbg != 0)
      {
        Moose::out << "getoutputmolspeciesphase: " << _sp_phase_name[i] << " "
                   << _sp_species_name[i] << " " << idbg << "\n";
      }
      else
      {
        // Moose::out << _ph_name[i].c_str() << " ZR: " << _zr_conc_phase[_current_node->id()][i] <<
        // "\n";
      }
    }

    if (_output_element_potential)
    {
      _element_potential_for_output[_current_node->id()].resize(_element_potentials.size());
      for (unsigned int i = 0; i < _element_potentials.size(); i++)
      {
        char cElName[3];
        _element_potential_for_output[_current_node->id()][i] = 0.0;
        Thermochimica::ConvertToFortran(cElName, sizeof cElName, _element_potentials[i].c_str());
        FORTRAN_CALL(Thermochimica::getoutputchempot)
        (cElName, &_element_potential_for_output[_current_node->id()][i], &idbg);
        if (idbg == -1)
        {
          Moose::out << "getoutputchempot " << idbg << "\n";
        }
        else if (idbg == 1)
        {
          // element not present, just leave this at 0 for now
          _element_potential_for_output[_current_node->id()][i] = 0.0;
        }
      }
    }
  }

#endif
}

void
ThermochimicaNodalData::finalize()
{
}

void
ThermochimicaNodalData::threadJoin(const UserObject & y)
{
  (void)y;
}

// Function to get re-initialization data from Thermochimica and save it in member variables of this
// UserObject
void
ThermochimicaNodalData::reinitDataMooseFromTc()
{
#ifdef THERMOCHIMICA_ENABLED
  if (_reinit_requested > 0)
  {
    FORTRAN_CALL(Thermochimica::savereinitdata)();
    _elements[_current_node->id()] = 0;
    _species[_current_node->id()] = 0;
    FORTRAN_CALL(Thermochimica::getreinitdatasizes)
    (&_elements[_current_node->id()], &_species[_current_node->id()]);
    _assemblage[_current_node->id()].resize(_elements[_current_node->id()]);
    _moles_phase[_current_node->id()].resize(_elements[_current_node->id()]);
    _element_potential[_current_node->id()].resize(_elements[_current_node->id()]);
    _elements_used[_current_node->id()].resize(169);
    _chemical_potential[_current_node->id()].resize(_species[_current_node->id()]);
    _mol_fraction[_current_node->id()].resize(_species[_current_node->id()]);
    FORTRAN_CALL(Thermochimica::reinitdatatctomoose)
    (&_assemblage[_current_node->id()][0],
     &_moles_phase[_current_node->id()][0],
     &_element_potential[_current_node->id()][0],
     &_chemical_potential[_current_node->id()][0],
     &_mol_fraction[_current_node->id()][0],
     &_elements_used[_current_node->id()][0],
     &_reinit_available[_current_node->id()]);
  }

  else
  {
    // If phase concentration data output has been requested, _moles_phase is required even if other
    // re-initialization data is not
    if (_n_phases > 0)
    {
      _elements[_current_node->id()] = 0;
      FORTRAN_CALL(Thermochimica::getreinitdatasizes)
      (&_elements[_current_node->id()], &_species[_current_node->id()]);
      _moles_phase[_current_node->id()].resize(_elements[_current_node->id()]);
      FORTRAN_CALL(Thermochimica::getmolesphase)(&_moles_phase[_current_node->id()][0]);
    }
    // // If element chemical potential data output has been requested, _element_potential is required even if other re-initialization data is not
    // if (_output_element_potential)
    // {
    //   _elements[_current_node->id()] = 0;
    //   FORTRAN_CALL(Thermochimica::getreinitdatasizes)(&_elements[_current_node->id()],&_species[_current_node->id()]);
    //   _element_potential[_current_node->id()].resize(_elements[_current_node->id()]);
    //   int idbg = 0;
    //   for (int i = 1; i <= _elements[_current_node->id()]; i++)
    //   {
    //     FORTRAN_CALL(Thermochimica::getallelementpotential)(&_element_potential[_current_node->id()][0])
    //   }
    // }
  }
#endif
}

// Function to load re-initialization data saved in this UserObject back into Thermochimica
void
ThermochimicaNodalData::reinitDataMooseToTc()
{
#ifdef THERMOCHIMICA_ENABLED
  // Tell Thermochimica whether a re-initialization is requested for this calculation
  FORTRAN_CALL(Thermochimica::setreinitrequested)(&_reinit_requested);
  // If we have re-initialization data and want a re-initialization, then load data into
  // Thermochimica
  if ((_elements[_current_node->id()] > 0) && (_reinit_requested > 0) &&
      (_reinit_available[_current_node->id()] > 0))
  {
    FORTRAN_CALL(Thermochimica::reinitdatatcfrommoose)
    (&_elements[_current_node->id()],
     &_species[_current_node->id()],
     &_assemblage[_current_node->id()][0],
     &_moles_phase[_current_node->id()][0],
     &_element_potential[_current_node->id()][0],
     &_chemical_potential[_current_node->id()][0],
     &_mol_fraction[_current_node->id()][0],
     &_elements_used[_current_node->id()][0]);
  }
#endif
}
