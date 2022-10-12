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

#pragma once

// MOOSE includes
#include "NodalUserObject.h"

class ThermochimicaNodalData : public NodalUserObject
{
public:
  static InputParameters validParams();
  ThermochimicaNodalData(const InputParameters & parameters);

  // Standard UserObject functions
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  // Thermochimica reinit functions
  void reinitDataMooseFromTc();
  void reinitDataMooseToTc();

  //  Accessor functions
  std::vector<double> getMolesPhase(dof_id_type node_id) const { return _moles_phase.at(node_id); }
  std::vector<int> getPhaseIndices(dof_id_type node_id) const { return _phase_indices.at(node_id); }
  std::vector<int> getAssemblage(dof_id_type node_id) const { return _assemblage.at(node_id); }
  std::vector<Real> getSpeciesFractions(dof_id_type node_id) const
  {
    return _species_fractions.at(node_id);
  }
  std::vector<double> getElementPotential(dof_id_type node_id) const
  {
    return _element_potential_for_output.at(node_id);
  }

protected:
  std::vector<const VariableValue *> _el;
  std::vector<std::string> _el_name;
  unsigned int _n_phases;
  std::vector<std::string> _ph_name;
  unsigned int _n_species;
  std::vector<std::string> _sp_phase_name;
  std::vector<std::string> _sp_species_name;

  Real _pressure;
  const VariableValue & _temp;

  // re-initialization data
  int _reinit_requested;
  std::unordered_map<dof_id_type, int> _reinit_available;
  std::unordered_map<dof_id_type, int> _elements;
  std::unordered_map<dof_id_type, int> _species;
  std::unordered_map<dof_id_type, std::vector<int>> _elements_used;
  std::unordered_map<dof_id_type, std::vector<int>> _assemblage;
  std::unordered_map<dof_id_type, std::vector<double>> _moles_phase;
  std::unordered_map<dof_id_type, std::vector<double>> _element_potential;
  std::unordered_map<dof_id_type, std::vector<double>> _chemical_potential;
  std::unordered_map<dof_id_type, std::vector<double>> _mol_fraction;
  std::unordered_map<dof_id_type, std::vector<Real>> _species_fractions;

  // Phase data
  const bool _phases_coupled;
  const bool _species_coupled;
  std::unordered_map<dof_id_type, std::vector<int>> _phase_indices;

  // Element chemical potential output
  const bool _output_element_potential;
  std::vector<std::string> _element_potentials;
  std::unordered_map<dof_id_type, std::vector<double>> _element_potential_for_output;
};
