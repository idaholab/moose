//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "ThermochimicaNodalData.h"

/**
 * AuxKernel to extract data from a ThermochimicaNodalData user object
 * into multiple (coupled) AuxVariables. The variable this AuxKernel is
 * operating on is set to the number of stable phases.
 */
class ThermochimicaAux : public AuxKernel
{
public:
  static InputParameters validParams();
  ThermochimicaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Total number of phases to output
  const std::size_t _n_phases;
  /// Writable phase amount variables
  std::vector<MooseVariable *> _ph;

  /// Total number of species to output
  const std::size_t _n_species;
  /// Writable species amount variables
  std::vector<MooseVariable *> _sp;

  /// Total number of vapor species
  const std::size_t _n_vapor_species;
  /// Writable vapour pressures for each element
  std::vector<MooseVariable *> _vapor_pressures;

  /// Total number of elements to output
  const std::size_t _n_elements;
  /// Writable chemical potential variables for each element
  std::vector<MooseVariable *> _el_pot;

private:
#ifdef THERMOCHIMICA_ENABLED
  /// User object to pull the data from
  const ThermochimicaNodalData & _thermo_nodal_data_uo;
#endif
};
