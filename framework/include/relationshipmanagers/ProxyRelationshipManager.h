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
#include "libmesh/ghosting_functor.h"

class MooseMesh;
namespace libMesh
{
class System;
}

/**
 * Intermediate base class for RelationshipManagers that are simply built
 * using ghosting functors.  The functor should be built in internalInitWithMesh()
 * and set as _functor
 */
class ProxyRelationshipManager : public RelationshipManager
{
public:
  static InputParameters validParams();

  ProxyRelationshipManager(const InputParameters & parameters);

  ProxyRelationshipManager(const ProxyRelationshipManager & other);

  virtual void operator()(const MeshBase::const_element_iterator & /*range_begin*/,
                          const MeshBase::const_element_iterator & /*range_end*/,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  virtual std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & /*rhs*/) const override;

  /**
   * A clone() is needed because GhostingFunctor can not be shared between
   * different meshes. The operations in  GhostingFunctor are mesh dependent.
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override{};

  libMesh::System * _other_system;
};
