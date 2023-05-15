//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodalUserObject.h"

/**
 * User object that performs a Gibbs energy minimization at each node by calling
 * the Thermochimica code.
 */
class ThermochimicaNodalData : public NodalUserObject
{
public:
  static InputParameters validParams();
  ThermochimicaNodalData(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override {}

  /**
   * Function to get re-initialization data from Thermochimica and save
   * it in member variables of this UserObject.
   */
  void reinitDataMooseFromTc();

  /**
   * Function to load re-initialization data saved in this UserObject
   * back into Thermochimica.
   */
  void reinitDataMooseToTc();

  struct Data
  {
    int _reinit_available;
    int _elements;
    int _species;
    std::vector<int> _elements_used;
    std::vector<int> _assemblage;
    std::vector<double> _moles_phase;
    std::vector<double> _element_potential;
    std::vector<double> _chemical_potential;
    std::vector<double> _mol_fraction;
    std::vector<Real> _species_fractions;
    std::vector<Real> _vapor_pressures;
    std::vector<int> _phase_indices;
    std::vector<double> _element_potential_for_output;
  };

  const Data & getNodalData(dof_id_type node_id) const;

protected:
  const VariableValue & _pressure;
  const VariableValue & _temperature;

  // re-initialization data
  const bool _reinit_requested;

  const std::size_t _n_phases;
  const std::size_t _n_species;
  const std::size_t _n_elements;
  const std::size_t _n_vapor_species;

  std::vector<const VariableValue *> _el;
  std::vector<std::string> _el_name;
  std::vector<unsigned int> _el_id;

  std::pair<int, int> _db_num_phases;
  std::vector<std::string> _db_phase_names;
  std::vector<std::vector<std::string>> _db_species_names;

  std::vector<std::string> _ph_name;
  std::vector<std::string> _sp_phase_name;
  std::vector<std::string> _sp_species_name;
  std::vector<std::string> _vapor_phase_name;
  std::vector<std::string> _vapor_species_name;

  /// Nodal data (TODO: investigate writing directly to AuxVariables)
  std::unordered_map<dof_id_type, Data> _data;

  ///@{ Element chemical potential output
  const bool _output_element_potential;
  const bool _output_vapor_pressures;
  std::vector<std::string> _element_potentials;
  ///@}
};
