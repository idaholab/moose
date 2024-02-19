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
#include "ElementUserObject.h"

// Forward declaration
class ChemicalCompositionAction;

template <bool is_nodal>
using ThermochimicaDataBaseParent =
    typename std::conditional<is_nodal, NodalUserObject, ElementUserObject>::type;

/**
 * User object that performs a Gibbs energy minimization at each node by
 * calling the Thermochimica code. This object can only be added through
 * the ChemicalCompositionAction which also sets up the variables for the
 * calculation.
 */
template <bool is_nodal>
class ThermochimicaDataBase : public ThermochimicaDataBaseParent<is_nodal>
{
public:
  static InputParameters validParams();
  ThermochimicaDataBase(const InputParameters & parameters);
  ~ThermochimicaDataBase();

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

  const Data & getNodalData(dof_id_type id) const;
  const Data & getElementData(dof_id_type id) const;

  // universal data access
  const Data & getData(dof_id_type id) const;

protected:
  // child process routine to dispatch thermochimica calculations
  void server();

  // helper to wait on a socket read
  template <typename T>
  void expect(T expect_msg);

  // send message to socket
  template <typename T>
  void notify(T send_msg);

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

  /// Nodal data (Used only for reinitialization)
  std::unordered_map<dof_id_type, Data> _data;

  ///@{ Element chemical potential output
  const bool _output_element_potentials;
  const bool _output_vapor_pressures;
  const bool _output_element_phases;
  ///@}

  /// Writable phase amount variables
  std::vector<MooseWritableVariable *> _ph;

  /// Writable species amount variables
  std::vector<MooseWritableVariable *> _sp;

  /// Writable vapour pressures for each element
  std::vector<MooseWritableVariable *> _vp;

  /// Writable chemical potential variables for each element
  std::vector<MooseWritableVariable *> _el_pot;

  /// Writable variable for molar amounts of each element in specified phase
  std::vector<MooseWritableVariable *> _el_ph;

  /// Mass unit for output species
  const enum class OutputMassUnit { MOLES, FRACTION } _output_mass_unit;

  /// communication socket
  int _socket;

  /// child PID
  pid_t _pid;

  /// shared memory pointer for dof_id_type values
  dof_id_type * _shared_dofid_mem;

  /// shared memory pointer for Real values
  Real * _shared_real_mem;

  // current node or element ID
  dof_id_type _current_id;

  using ThermochimicaDataBaseParent<is_nodal>::isCoupled;
  using ThermochimicaDataBaseParent<is_nodal>::isParamValid;
  using ThermochimicaDataBaseParent<is_nodal>::coupledValue;
  using ThermochimicaDataBaseParent<is_nodal>::coupledComponents;
  using ThermochimicaDataBaseParent<is_nodal>::writableVariable;
};

typedef ThermochimicaDataBase<true> ThermochimicaNodalData;
typedef ThermochimicaDataBase<false> ThermochimicaElementData;
