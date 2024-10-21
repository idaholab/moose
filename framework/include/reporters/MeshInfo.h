//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

namespace libMesh
{
class EquationSystems;
class System;
class MeshBase;
}

/**
 * Report mesh information, such as the number of elements, nodes, and degrees of freedom.
 */
class MeshInfo : public GeneralReporter
{
public:
  static InputParameters validParams();
  MeshInfo(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

  /**
   * Helper struct for defining information about a single sideset.
   */
  struct SidesetInfo
  {
    BoundaryID id;
    std::string name;
    std::vector<std::pair<dof_id_type, unsigned int>> sides;
  };

  /**
   * Helper struct for defining information about a single subdomain.
   */
  struct SubdomainInfo
  {
    BoundaryID id;
    std::string name;
    std::vector<dof_id_type> elems;
  };

protected:
  const MultiMooseEnum & _items;

  // Reporter values to return
  unsigned int & _num_dofs;
  unsigned int & _num_dofs_nl;
  unsigned int & _num_dofs_aux;
  unsigned int & _num_dofs_constrained;
  unsigned int & _num_elem;
  unsigned int & _num_node;
  unsigned int & _num_local_dofs;
  unsigned int & _num_local_dofs_nl;
  unsigned int & _num_local_dofs_aux;
  unsigned int & _num_local_elem;
  unsigned int & _num_local_node;
  std::map<BoundaryID, SidesetInfo> & _local_sidesets;
  std::map<BoundaryID, SidesetInfo> & _local_sideset_elems;
  std::map<BoundaryID, SidesetInfo> & _sidesets;
  std::map<BoundaryID, SidesetInfo> & _sideset_elems;
  std::map<SubdomainID, SubdomainInfo> & _local_subdomains;
  std::map<SubdomainID, SubdomainInfo> & _local_subdomain_elems;
  std::map<SubdomainID, SubdomainInfo> & _subdomains;
  std::map<SubdomainID, SubdomainInfo> & _subdomain_elems;

  // Helper to perform optional declaration based on "_items"
  template <typename T>
  T & declareHelper(const std::string & item_name, const ReporterMode mode);

private:
  /// Possibly add to _local_sidesets, _local_sideset_elems, _sidesets, and _sideset_elems
  void possiblyAddSidesetInfo();
  /// Possibly add to _local_subdomains, _local_subdomain_elems, _subdomains, and _subdomain_elems
  void possiblyAddSubdomainInfo();

  const libMesh::EquationSystems & _equation_systems;
  const libMesh::System & _nonlinear_system;
  const libMesh::System & _aux_system;
  const libMesh::MeshBase & _mesh;
};

template <typename T>
T &
MeshInfo::declareHelper(const std::string & item_name, const ReporterMode mode)
{
  return (!_items.isValid() || _items.isValueSet(item_name))
             ? declareValueByName<T>(item_name, mode)
             : declareUnusedValue<T>();
}

void to_json(nlohmann::json & json, const std::map<BoundaryID, MeshInfo::SidesetInfo> & sidesets);
void dataStore(std::ostream & stream, MeshInfo::SidesetInfo & sideset_info, void * context);
void dataLoad(std::istream & stream, MeshInfo::SidesetInfo & sideset_info, void * context);

void to_json(nlohmann::json & json, const std::map<BoundaryID, MeshInfo::SubdomainInfo> & sidesets);
void dataStore(std::ostream & stream, MeshInfo::SubdomainInfo & sideset_info, void * context);
void dataLoad(std::istream & stream, MeshInfo::SubdomainInfo & sideset_info, void * context);
