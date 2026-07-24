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
  mfem::Mesh generateMFEMMesh() override;

private:
  const unsigned int _dim;
  const unsigned int _nx;
  const unsigned int _ny;
  const unsigned int _nz;
  const Real _xmax;
  const Real _ymax;
  const Real _zmax;
};

#endif
