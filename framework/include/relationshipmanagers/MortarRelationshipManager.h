//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// App includes
#include "AutomaticMortarGeneration.h"
#include "RelationshipManager.h"

// libMesh includes
#include "libmesh/mesh_base.h"

using libMesh::boundary_id_type;
using libMesh::Elem;
using libMesh::GhostingFunctor;
using libMesh::MeshBase;
using libMesh::processor_id_type;

class MortarRelationshipManager;

template <>
InputParameters validParams<MortarRelationshipManager>();

class MortarRelationshipManager : public RelationshipManager
{
public:
  MortarRelationshipManager(const InputParameters &);

  static InputParameters validParams();

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  virtual void mesh_reinit() override;

  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator==(const RelationshipManager & other) const override;

protected:
  void internalInit() override {}

  BoundaryName _master_boundary_name;
  BoundaryName _slave_boundary_name;
  SubdomainName _master_subdomain_name;
  SubdomainName _slave_subdomain_name;
};
