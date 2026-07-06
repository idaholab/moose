//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// libMesh includes
#include "libmesh/petsc_vector.h"

// MOOSE includes
#include "NEML2PostKernel.h"

/**
 * Batched hourglass stabilization for underintegrated (single quadrature point)
 * QUAD4 (2 displacements, incl. RZ) and HEX8 (3 displacements) elements in the
 * NEML2 nodal-force path. Per element and displacement component, the best-fit
 * affine part of the nodal displacement is removed and the non-affine hourglass
 * modes are penalized with a rotation-invariant scale, following
 * HourglassCorrectionQuad4 (Flanagan-Belytschko-style control with a
 * least-squares affine fit). Operates on the displaced geometry.
 */
class NEML2HourglassCorrection : public NEML2PostKernel
{
public:
  static InputParameters validParams();

  NEML2HourglassCorrection(const InputParameters & parameters);

protected:
  void forward() override;

  /// Residual vector to assemble into
  libMesh::PetscVector<Real> * _residual;

  /// Base penalty parameter
  const Real _penalty;

  /// Shear modulus scaling the stabilization
  const Real _mu;

  /// Displacement variables
  const std::vector<NonlinearVariableName> _disp_vars;

  /// Number of displacement variables (2 = QUAD4, 3 = HEX8)
  const unsigned int _ndisp;

  /// Whether the mesh is axisymmetric (RZ)
  const bool _rz;

  /// Index of the radial coordinate under RZ
  const unsigned int _radial_coord;

  /// Reference node coordinates (nelem, nnode, 3)
  const neml2::Tensor & _node_coords;

  /// Nodal displacement values per component (nelem, nnode)
  std::vector<const neml2::Tensor *> _disp_nodal;

  /// Hourglass mode vectors (nmode, nnode), set per element family
  neml2::Tensor _gamma;
};

#endif
