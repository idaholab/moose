//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusUELUserObjectBase.h"
#include "BlockRestrictable.h"

class MooseMesh;
namespace libMesh
{
class MeshBase;
}

/**
 * This user-object is a testbed for implementing a custom element.
 */
class AbaqusUserElement : public AbaqusUELUserObjectBase<dof_id_type, std::map>,
                          public BlockRestrictable
{
public:
  static InputParameters validParams();
  AbaqusUserElement(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void meshChanged() override;

  virtual void initialize() override final;
  virtual void execute() override;
  virtual void finalize() override final {}

  /// getters for the loop class
  const std::vector<const MooseVariableFieldBase *> & getVariables() const { return _variables; }
  const std::vector<const MooseVariableFieldBase *> & getAuxVariables() const
  {
    return _aux_variables;
  }

  const uel_t & getPlugin() const { return _uel; }

  const Real & getPNewDt() const { return _pnewdt; }

protected:
  /// setup the range of elements this object operates on
  void setupElemRange();

  /// The \p MooseMesh that this user object operates on
  MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// coupled variables to provide the DOF values
  std::vector<NonlinearVariableName> _variable_names;

  /// Auxiliary variable names
  std::vector<AuxVariableName> _aux_variable_names;

  /// pointers to the variable objects
  std::vector<const MooseVariableFieldBase *> _variables;

  /// pointers to the auxiliary variable objects
  std::vector<const MooseVariableFieldBase *> _aux_variables;

  /// The subdomain ids this object operates on
  const std::set<SubdomainID> _sub_ids;

  /// All the active and elements local to this process that exist on this object's subdomains
  std::unique_ptr<ConstElemRange> _elem_range;

  /// props
  std::vector<Real> _props;
  int _nprops;

  /// Abaqus element type
  const int _jtype;

  friend class UELThread;
};
