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

#include "MFEMMeshGenerator.h"

/**
 * Generates a structured Cartesian MFEM mesh: a line (1D), rectangle (2D), or
 * box (3D) with uniformly spaced elements. Analogous to GeneratedMeshGenerator
 * for the MFEM mesh pipeline.
 */
class MFEMGeneratedMeshGenerator : public MFEMMeshGenerator
{
public:
  static InputParameters validParams();

  MFEMGeneratedMeshGenerator(const InputParameters & parameters);

protected:
  std::unique_ptr<mfem::Mesh> generateMFEMMesh() override;

private:
  /// Mesh dimension (1, 2, or 3)
  const unsigned int _dim;
  /// Number of elements in the x direction
  const unsigned int _nx;
  /// Number of elements in the y direction
  const unsigned int _ny;
  /// Number of elements in the z direction
  const unsigned int _nz;
  /// Upper bound of the domain in the x direction (lower bound is 0)
  const Real _xmax;
  /// Upper bound of the domain in the y direction (lower bound is 0)
  const Real _ymax;
  /// Upper bound of the domain in the z direction (lower bound is 0)
  const Real _zmax;
  /// Element type (resolved from user input or dimension-based default); unused for dim == 1
  const mfem::Element::Type _elem_type;
};

#endif
