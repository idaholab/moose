// AbaqusUELMesh//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MoosePassKey.h"
#include "AbaqusInputParser.h"
#include "AbaqusInputObjects.h"

#include <list>
#include <stdexcept>

class AbaqusUELMeshUserElement;

/**
 * Coupling user object to use Abaqus UEXTERNALDB subroutines in MOOSE
 */
class AbaqusUELMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  AbaqusUELMesh(const InputParameters & parameters);
  AbaqusUELMesh(const AbaqusUELMesh & other_mesh);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  virtual bool prepare(const MeshBase * mesh_to_clone) override;

  // const std::vector<std::size_t> & getNodeSet(const std::string & elset) const;
  // const std::vector<std::size_t> & getElementSet(const std::string & elset) const;
  // const auto & getICBlocks() const { return _abaqus_ics; }
  // const auto & getProperties() const { return _properties; }

  std::string getVarName(std::size_t id) const;
  const Abaqus::UserElement & getUEL(const std::string & type) const;
  const auto & getUELs() const { return _model->_element_definition; }

  /// get a set of all SubdomainIDs used for restricting variable-node assignment
  const auto & getVarBlocks() const { return _uel_block_ids; }

  /// get a vector of all user elements
  const auto & getElements() const { return _model->_elements; }

  /// privileged write access
  auto & getElements(Moose::PassKey<AbaqusUELMeshUserElement>) { return _model->_elements; }

  /// get a vector of all nodes
  const auto & getNodes() const { return _model->_nodes; }

  /// get a map of all nodes to user elements
  const auto & getNodeToUELMap() const { return _node_to_uel_map; }

  const auto & getElementSets() const { return _model->_elsets; }
  const auto & getNodeSets() const { return _model->_nsets; }

  // initial condition getters
  const auto & getFieldICs() const { return _model->_field_ics; }

  void addNodeset(BoundaryID id);

protected:
  Abaqus::InputParser _input;
  std::unique_ptr<Abaqus::Model> _model;

  void instantiateElements();
  void setupLibmeshSubdomains();
  void setupNodeSets();

  dof_id_type _max_node_id;

  /// A map from nodes (i.e. node elements) to user elements (index into _model->_elements)
  /// libMesh node IDs are AbaqusIDs.
  std::unordered_map<dof_id_type, std::vector<Abaqus::Index>> _node_to_uel_map;

  /// all subdomain IDs used for UEL variable restriction
  std::set<SubdomainID> _uel_block_ids;

  /// UEL node sets
  std::map<std::string, std::vector<std::size_t>> _node_set;

  /// UEL element sets
  std::map<std::string, std::vector<std::size_t>> _element_set;

  /// variable names (zero based indexing)
  std::vector<std::string> _var_names;

  /// enable additional debugging output
  const bool _debug;
};
