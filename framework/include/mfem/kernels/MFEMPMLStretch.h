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

#include "MFEMKernel.h"

/**
 * Radial perfectly matched layer coordinate stretch about a reference point.
 *
 * Depth into the layer is measured along the straight line from the reference point p0 through the
 * evaluation point. That ray meets the inner surface of the layer at radius r_in and the outer
 * surface at r_out, giving a layer thickness L = r_out - r_in and a depth s = |x - p0| - r_in.
 * Writing the stretched radius as r~ = r + i g(s), the stretch is diagonal in the local radial and
 * tangential frame with factors
 *
 *     J_r = 1 + i n alpha s^(n-1) / L^n     (radial, dr~/dr)
 *     J_t = 1 + i alpha s^n / (L^n r)       (tangential, r~/r)
 *
 * where alpha is the decay coefficient and n the decay polynomial order. The mesh must either be
 * convex or the chosen reference point must be such that the ray from the reference point through
 * any evaluation point intersects the inner and outer surfaces of the layer exactly once.
 */
class MFEMPMLStretch
{
public:
  /// An empty reference_point selects the barycenter of the mesh.
  MFEMPMLStretch(mfem::ParMesh & mesh,
                 const mfem::Array<int> & pml_attributes,
                 const mfem::Vector & reference_point,
                 double decay_coefficient,
                 double decay_polynomial,
                 MPI_Comm comm);

  int dim() const { return _dim; }

  /// Unit radial direction and the radial and tangential stretch factors at a point.
  void evaluate(const mfem::Vector & x,
                mfem::Vector & radial_direction,
                std::complex<double> & radial_factor,
                std::complex<double> & tangential_factor) const;

private:
  /// Volume weighted centroid of the mesh, reduced across ranks.
  mfem::Vector meshBarycenter(mfem::ParMesh & mesh, MPI_Comm comm) const;

  /// Distance from the reference point to the nearest intersection of the ray with the given
  /// surface. Returns a negative value if the ray misses every face.
  double castRay(const mfem::Vector & direction, const std::vector<double> & faces) const;

  const int _dim;
  mfem::Vector _reference_point;
  const double _decay_coefficient;
  const double _decay_polynomial;
  /// Flattened face vertex coordinates: two points per segment in 2D, three per triangle in 3D.
  std::vector<double> _inner_faces;
  std::vector<double> _outer_faces;
};

#endif
