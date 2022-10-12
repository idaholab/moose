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

class ThermochimicaAux : public AuxKernel
{
public:
  static InputParameters validParams();
  ThermochimicaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const std::size_t _n_phases;
  std::vector<VariableValue *> _ph;
  std::vector<std::string> _ph_name;

  const std::size_t _n_species;
  std::vector<VariableValue *> _sp;
  std::vector<std::string> _sp_phase_name;
  std::vector<std::string> _sp_species_name;

  const std::size_t _n_elements;
  std::vector<VariableValue *> _el_pot;

private:
  const ThermochimicaNodalData * _thermo_nodal_data_uo;
};
