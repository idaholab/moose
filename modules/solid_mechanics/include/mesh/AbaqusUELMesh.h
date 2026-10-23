//* This file is part of the MOOSE framework
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

class AbaqusUELMeshUserElement;

/**
 * Mesh that reads an Abaqus input with user elements and builds one NodeElem per UEL node
 */
class AbaqusUELMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  AbaqusUELMesh(const InputParameters & parameters);
  AbaqusUELMesh(const AbaqusUELMesh & other_mesh);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  virtual void prepare() override;

  std::string getVarName(const Abaqus::AbaqusID var_id) const;
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

  const auto & getSteps() const { return _model->_step; }
  const auto & getModel() const { return *_model; }

  /// initial condition getters
  const auto & getFieldICs() const { return _model->_field_ics; }

protected:
  /// parsed Abaqus model, shared with clones of this mesh (e.g. the displaced mesh)
  std::shared_ptr<Abaqus::Model> _model;

  void instantiateElements();
  void buildNodeToUELMap();
  void setupNodeSets();

  /// A map from nodes (i.e. node elements) to user elements (index into _model->_elements)
  /// libMesh node IDs are AbaqusIDs.
  std::unordered_map<dof_id_type, std::vector<Abaqus::Index>> _node_to_uel_map;

  /// all subdomain IDs used for UEL variable restriction
  std::set<SubdomainID> _uel_block_ids;
};
