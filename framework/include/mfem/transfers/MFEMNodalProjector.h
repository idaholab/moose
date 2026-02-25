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

#include "mfem.hpp"

/**
 * Auxiliary class to extract locations of nodes in MFEM GridFunctions
 * and project values defined at them to set DoFs
 */
class MFEMNodalProjector
{
public:
  MFEMNodalProjector() = default;
  /// Extract node positions from MFEM FESpace at which projection will take place
  void extractNodePositions(const mfem::ParFiniteElementSpace & fespace, mfem::Vector & node_positions, mfem::Ordering::Type & node_ordering);
  /// Project a vector of values provided at projection points (nodes) to set GridFunction DoFs
  void projectNodalValues(const mfem::Vector & nodal_vals, const mfem::Ordering::Type & nodal_val_ordering, mfem::ParGridFunction & gridfunction);
};

#endif
