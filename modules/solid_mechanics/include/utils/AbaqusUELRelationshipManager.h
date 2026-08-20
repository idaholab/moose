//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RelationshipManager.h"
#include <memory>

class AbaqusUELMesh;

/**
 * This class implements algebraic coupling fur Abaqus user defined elements (UELs).
 */
class AbaqusUELRelationshipManager : public RelationshipManager
{
public:
  AbaqusUELRelationshipManager(const InputParameters & params);

  AbaqusUELRelationshipManager(const AbaqusUELRelationshipManager & other);

  static InputParameters validParams();

  /**
   * For the specified range of active elements, find the elements
   * which will be coupled to them in the sparsity pattern.
   *
   * This will be the node elements in all UEL elements that include the node
   */
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  /**
   * A clone() is needed because GhostingFunctor can not be shared between
   * different meshes. The operations in  GhostingFunctor are mesh dependent.
   */
  virtual std::unique_ptr<libMesh::GhostingFunctor> clone() const override;

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;

private:
  AbaqusUELMesh * _uel_mesh;
};
