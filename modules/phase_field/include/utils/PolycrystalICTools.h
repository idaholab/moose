//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "libmesh/libmesh.h"
#include "InitialCondition.h"

namespace PolycrystalICTools
{

/**
 * Simple 2D block matrix indicating graph adjacency. We use a 1D
 * storage structure so that we can  pass it to PETSc easily.
 */
template <typename T>
class AdjacencyMatrix
{
public:
  AdjacencyMatrix(unsigned int size) : _size(size), _data(size * size) {}

  ~AdjacencyMatrix() = default;

  // Get rid of copy constructors
  AdjacencyMatrix(const AdjacencyMatrix & f) = delete;
  AdjacencyMatrix & operator=(const AdjacencyMatrix & f) = delete;

  // Use only move constructors
  AdjacencyMatrix(AdjacencyMatrix && /* f */) = default;
  AdjacencyMatrix & operator=(AdjacencyMatrix && /* f */) = default;

  T & operator()(unsigned int i, unsigned int j) { return _data[i * _size + j]; }
  T operator()(unsigned int i, unsigned int j) const { return _data[i * _size + j]; }

  std::size_t size() const { return _size; }
  T * rawDataPtr() { return _data.data(); }

private:
  const std::size_t _size;
  std::vector<T> _data;
};

std::vector<unsigned int> assignPointsToVariables(const std::vector<Point> & centerpoints,
                                                  const Real op_num,
                                                  const MooseMesh & mesh,
                                                  const MooseVariable & var);

unsigned int assignPointToGrain(const Point & p,
                                const std::vector<Point> & centerpoints,
                                const MooseMesh & mesh,
                                const MooseVariable & var,
                                const Real maxsize);

AdjacencyMatrix<Real>
buildGrainAdjacencyMatrix(const std::map<dof_id_type, unsigned int> & entity_to_grain,
                          MooseMesh & mesh,
                          const libMesh::PeriodicBoundaries * pb,
                          unsigned int n_grains,
                          bool is_elemental);

AdjacencyMatrix<Real>
buildElementalGrainAdjacencyMatrix(const std::map<dof_id_type, unsigned int> & element_to_grain,
                                   MooseMesh & mesh,
                                   const libMesh::PeriodicBoundaries * pb,
                                   unsigned int n_grains);

AdjacencyMatrix<Real>
buildNodalGrainAdjacencyMatrix(const std::map<dof_id_type, unsigned int> & node_to_grain,
                               MooseMesh & mesh,
                               const libMesh::PeriodicBoundaries * pb,
                               unsigned int n_grains);

std::vector<unsigned int> assignOpsToGrains(AdjacencyMatrix<Real> & adjacency_matrix,
                                            unsigned int n_grains,
                                            unsigned int n_ops,
                                            const MooseEnum & coloring_algorithm);

MooseEnum coloringAlgorithms();

std::string coloringAlgorithmDescriptions();
}
