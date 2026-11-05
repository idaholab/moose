//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusInputObjects.h"
#include "AbaqusUELUserObjectBase.h"

#include <unordered_map>

class AbaqusUELMesh;
class AbaqusDLoadInterpolator;

/**
 * Calls an Abaqus UEL plugin on the user elements of the selected AbaqusUELMesh element sets and
 * assembles their residual and Jacobian contributions.
 */
class AbaqusUELMeshUserElement : public AbaqusUELUserObjectBase<Abaqus::Index, std::unordered_map>
{
public:
  static InputParameters validParams();
  AbaqusUELMeshUserElement(const InputParameters & params);

  virtual void initialize() override final {}
  virtual void execute() override;
  virtual void finalize() override final {}

  std::pair<std::size_t, const std::unordered_map<Abaqus::Index, std::vector<Real>> &>
  getStateVars() const
  {
    return {_nstatev, _statev[_statev_index_current]};
  }
  const std::vector<Abaqus::Index> & getActiveElements() const { return _active_elements; }
  const std::vector<Abaqus::Element> & getElements() const { return _uel_elements; }

protected:
  /// setup the range of elements this object operates on
  void setupElementSet();

  /// the AbaqusUELMesh of the problem, needed before the members are initialized
  static AbaqusUELMesh & getUELMesh(const InputParameters & params);

  /// Auxiliary system object
  AuxiliarySystem & _aux_sys;

  /// UEL type name
  const std::string _uel_type;

  /// The \p MooseMesh that this user object operates on
  AbaqusUELMesh & _uel_mesh;

  /// definition of the UEL this object is operating on
  const Abaqus::UserElement & _uel_definition;

  /// all elements in the UEL mesh
  const std::vector<Abaqus::Element> & _uel_elements;

  /// selected set names
  const std::vector<std::string> & _element_set_names;

  /// active elements for each element set
  std::vector<Abaqus::Index> _active_elements;

  /// Auxiliary variable names
  const std::vector<AuxVariableName> _aux_variable_names;

  /// pointers to the variable objects
  std::vector<std::vector<const MooseVariableFieldBase *>> _variables;

  /// pointers to the auxiliary variable objects
  std::vector<const MooseVariableFieldBase *> _aux_variables;

  /// Residual contribution returned from the UEL plugin
  DenseVector<Real> _local_re;
  /// Jacobian contribution returned from the UEL plugin
  DenseMatrix<Real> _local_ke;
  /// transpose of the Jacobian inserted into the MOOSE non-linear system
  DenseMatrix<Real> _local_ke_T;

  /// DLOAD interpolator UO
  const AbaqusDLoadInterpolator & _dload_uo;
};
