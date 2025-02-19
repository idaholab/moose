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

  //  const auto & getVarBlocks() const { return _uel_block_ids; }
  // const auto & getUELs() const { return _element_definition; }
  // const auto & getElements() const { return _elements; }
  // const std::vector<std::size_t> & getNodeSet(const std::string & elset) const;
  // const std::vector<std::size_t> & getElementSet(const std::string & elset) const;
  // const auto & getElementSets() const { return _element_set; }
  // const auto & getNodeToUELMap() const { return _node_to_uel_map; }
  // const auto & getICBlocks() const { return _abaqus_ics; }
  // const auto & getProperties() const { return _properties; }

  /// privileged write access
  auto & getElements(Moose::PassKey<AbaqusUELMeshUserElement>) { return _elements; }

  std::string getVarName(std::size_t id) const;
  const Abaqus::UserElement & getUEL(const std::string & type) const;

  void addNodeset(BoundaryID id);

  /// The instantiation of Abaqus::Part::Element
  struct LibMeshUElement
  {
    LibMeshUElement(Abaqus::UserElement & uel)
      : _uel(uel), _pid(DofObject::invalid_processor_id), _properties({nullptr, nullptr})
    {
    }
    Abaqus::UserElement & _uel;
    processor_id_type _pid;
    std::vector<dof_id_type> _libmesh_node_list;
    std::pair<Real *, int *> _properties;
  };

protected:
  Abaqus::InputParser _input;
  Abaqus::Root _root;

  void instantiateElements();
  void setupLibmeshSubdomains();
  void setupNodeSets();

  dof_id_type _max_node_id;

  /// Element connectivity
  std::vector<LibMeshUElement> _elements;

  // /// A map from nodes (i.e. node elements) to user elements (ids)
  // std::unordered_map<dof_id_type, std::vector<int>> _node_to_uel_map;

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
