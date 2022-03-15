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

using libMesh::processor_id_type;

/**
 * Relationship manager to add ghosting between elements
 *
 * The list of element IDs and connected element IDs has to be constructed.
 * This is to be used when THM components need to see each other in distributed memory parallelism
 */
class AugmentSparsityBetweenElements : public RelationshipManager
{
public:
  AugmentSparsityBetweenElements(const InputParameters &);
  AugmentSparsityBetweenElements(const AugmentSparsityBetweenElements & others);

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;
  virtual std::unique_ptr<GhostingFunctor> clone() const override;
  virtual void mesh_reinit() override;
  virtual void redistribute() override;
  std::string getInfo() const override;
  virtual bool operator>=(const RelationshipManager & rhs) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  /// Map of element ghosting. Element IDs linked to list of element IDs that they are connected to
  const std::map<dof_id_type, std::vector<dof_id_type>> & _elem_map;

public:
  static InputParameters validParams();
};
