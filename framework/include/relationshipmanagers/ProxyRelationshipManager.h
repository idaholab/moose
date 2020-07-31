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

// Forward declarations
class ProxyRelationshipManager;
class MooseMesh;
namespace libMesh
{
class System;
}

template <>
InputParameters validParams<ProxyRelationshipManager>();

/**
 * Intermediate base class for RelationshipManagers that are simply built
 * using ghosting functors.  The functor should be built in internalInit()
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

  virtual bool operator==(const RelationshipManager & /*rhs*/) const override;

  virtual void set_mesh(const MeshBase * mesh) override
  {
    RelationshipManager::set_mesh(mesh);
    _this_mesh = mesh;
  }

  virtual std::unique_ptr<GhostingFunctor> clone() const override
  {
    return libmesh_make_unique<ProxyRelationshipManager>(*this);
  }

protected:
  virtual void internalInit() override{};

  const MeshBase * _this_mesh;

  System * _other_system;
};
