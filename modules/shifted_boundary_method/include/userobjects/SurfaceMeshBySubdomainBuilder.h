//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/mesh.h"
#include "KDTree.h"
#include "SBMBndElementBase.h"
#include "SBMBndEdge2.h"
#include "SBMBndTri3.h"
#include "SBMUtils.h"

class SurfaceMeshBySubdomainBuilder : public GeneralUserObject
{
public:
  static InputParameters validParams();
  SurfaceMeshBySubdomainBuilder(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  const std::unordered_map<subdomain_id_type, std::vector<Point>> & getCentroidsBySubdomain() const
  {
    return _centroids_by_subdomain;
  }

  const std::unordered_map<subdomain_id_type, std::vector<dof_id_type>> &
  getElemIdMapBySubdomain() const
  {
    return _elem_id_map_by_subdomain;
  }

  const std::unordered_map<subdomain_id_type, std::vector<std::unique_ptr<SBMBndElementBase>>> &
  getBoundaryElementsBySubdomain() const
  {
    return _boundary_elements_by_subdomain;
  }

protected:
  void buildSubdomainGroupedData();
  bool checkWatertightness() const;

  std::unique_ptr<MeshBase> _mesh;

  std::unordered_map<subdomain_id_type, std::vector<Point>> _centroids_by_subdomain;
  std::unordered_map<subdomain_id_type, std::vector<dof_id_type>> _elem_id_map_by_subdomain;
  std::unordered_map<subdomain_id_type, std::vector<std::unique_ptr<SBMBndElementBase>>>
      _boundary_elements_by_subdomain;

  int _leaf_max_size;
  std::string _bnd_mesh_name;
  bool _check_watertightness;
  unsigned int _dim_embedding_mesh;

  bool _check_replicated;
};
