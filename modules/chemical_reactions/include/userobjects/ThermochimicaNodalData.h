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

// Forward declaration
class ChemicalCompositionAction;

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
  };

  const Data & getNodalData(dof_id_type node_id) const;

protected:
  const VariableValue & _pressure;
  const VariableValue & _temperature;

  const std::size_t _n_phases;
  const std::size_t _n_species;
  const std::size_t _n_elements;
  const std::size_t _n_vapor_species;
  const std::size_t _n_phase_elements;
  const std::size_t _n_potentials;

  std::vector<const VariableValue *> _el;

  const ChemicalCompositionAction & _action;
  std::vector<unsigned int> _el_ids;

  // re-initialization data
  const enum class ReinitializationType { NONE, TIME, NODE } _reinit;

  const std::vector<std::string> & _ph_names;
  const std::vector<std::string> & _element_potentials;
  const std::vector<std::pair<std::string, std::string>> & _species_phase_pairs;
  const std::vector<std::pair<std::string, std::string>> & _vapor_phase_pairs;
  const std::vector<std::pair<std::string, std::string>> & _phase_element_pairs;

  /// Nodal data (TODO: investigate writing directly to AuxVariables)
  std::unordered_map<dof_id_type, Data> _data;

  ///@{ Element chemical potential output
  const bool _output_element_potentials;
  const bool _output_vapor_pressures;
  const bool _output_element_phases;
  ///@}

  /// Writable phase amount variables
  std::vector<MooseVariable *> _ph;

  /// Writable species amount variables
  std::vector<MooseVariable *> _sp;

  /// Writable vapour pressures for each element
  std::vector<MooseVariable *> _vp;

  /// Writable chemical potential variables for each element
  std::vector<MooseVariable *> _el_pot;

  /// Writable variable for molar amounts of each element in specified phase
  std::vector<MooseVariable *> _el_ph;

  /// Mass unit for output species
  const enum class OutputMassUnit { MOLES, FRACTION } _output_mass_unit;
};
