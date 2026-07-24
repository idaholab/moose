//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMSubdomainGeneratorBase.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe.h"
#include "libmesh/bounding_box.h"
#include "SBMBndElementBase.h"
#include "SBMBndEdge2.h"
#include "SBMBndTri3.h"
#include "PointInPolyhedronCheck.h"

class SubdomainGrainIDGenerator : public SBMSubdomainGeneratorBase
{
public:
  static InputParameters validParams();
  SubdomainGrainIDGenerator(const InputParameters & parameters);

  std::unique_ptr<libMesh::MeshBase> generate() override;

protected:
  struct SubdomainCheckerEntry
  {
    SubdomainID subdomain_id;
    BoundingBox bbox;
    PointInPolyhedronCheck * checker;
  };

  std::unique_ptr<libMesh::MeshBase> & _boundary_mesh;

protected:
  /// The centroids of the boundary elements grouped by subdomains
  std::unordered_map<SubdomainID, std::vector<Point>> _centroids_by_subdomain;
  /// The boundary elements grouped by subdomains
  std::unordered_map<SubdomainID, std::vector<std::unique_ptr<SBMBndElementBase>>>
      _boundary_elements_by_subdomain;
  /// The bounding boxes of the boundary elements grouped by subdomains
  std::unordered_map<SubdomainID, BoundingBox> _bboxes_by_subdomain;

  /// The IN-OUT testers for each subdomain
  std::unordered_map<SubdomainID, std::unique_ptr<PointInPolyhedronCheck>> _subdomain_id_checkers;
  /// Cache contiguous checker metadata for faster per-element scans
  std::vector<SubdomainCheckerEntry> _checker_entries;

  /// Ray shooting direction in Point format
  Point _ray_direction;

  /// Brute force means that we loop over all boundary elements to check if the point is inside.
  bool _brute_force;
  /// eps for intersection or on surface checking
  Real _eps;
  /// Maximum number of elements in a leaf node of the KD-tree
  int _leaf_max_size;

  /// The boundary mesh to use for identifying grain IDs
  std::unique_ptr<libMesh::MeshBase> _boundary_owned;

  /// build the data structure grouping by different subdomains
  void buildSubdomainGroupedData();

  /// Build the IN-OUT testers for each subdomain
  void buildInOutTesters();

  /// @brief Check whether two bounding boxes overlap within a small tolerance.
  static bool
  boundingBoxesIntersect(const BoundingBox & lhs, const BoundingBox & rhs, const Real tolerance);
};
