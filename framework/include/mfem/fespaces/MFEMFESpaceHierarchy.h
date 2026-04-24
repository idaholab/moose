//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/fem/fespacehierarchy.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Builds and owns a mfem::ParFiniteElementSpaceHierarchy from a base FESpace by
 * applying a sequence of h- and/or p-refinements.
 *
 * The `refinements` parameter is a vector of strings.  Each entry is either:
 *   - "h"  — add one uniformly h-refined level (same FEC, refined mesh)
 *   - "N"  — add one p-refined level at polynomial order N (N must be strictly
 *             greater than the current finest level's order)
 *
 * Total number of levels: refinements.size() + 1.
 *
 * The returned shared_ptr (getHierarchyShared()) may be co-owned by
 * Moose::MFEM::GeometricMultigridSolver and stored in ProblemData::fespace_hierarchies
 * to guarantee the hierarchy outlives any solver that references it.
 */
class MFEMFESpaceHierarchy : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMFESpaceHierarchy(const InputParameters & parameters);

  /// Returns a shared_ptr to the ParFiniteElementSpaceHierarchy. Safe for co-ownership.
  std::shared_ptr<mfem::ParFiniteElementSpaceHierarchy> getHierarchyShared() const
  {
    return _hierarchy;
  }

  /// Returns a reference to the ParFiniteElementSpaceHierarchy.
  mfem::ParFiniteElementSpaceHierarchy & getHierarchy() const { return *_hierarchy; }

  bool isScalar() const { return _is_scalar; }

  bool isVector() const { return _is_vector; }

private:
  void buildHierarchy();

  /// The hierarchy — shared_ptr so that GMS and ProblemData can co-own.
  std::shared_ptr<mfem::ParFiniteElementSpaceHierarchy> _hierarchy;

  /// FECs for p-refined levels. MFEM does not own them, so we keep them alive here.
  /// The lifetime of these is tied to this object which is kept alive by the MOOSE warehouse.
  std::vector<std::unique_ptr<mfem::FiniteElementCollection>> _level_fecs;

  bool _is_scalar = false;
  bool _is_vector = false;
};

#endif
