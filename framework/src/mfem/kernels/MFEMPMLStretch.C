//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLStretch.h"

namespace
{
/// Gather one surface's face geometry onto every rank. A ray starting from a local point may leave
/// through a face owned by another rank, so every rank needs the complete surface.
void
replicateFaces(std::vector<double> & faces, MPI_Comm comm)
{
  int comm_size = 0;
  MPI_Comm_size(comm, &comm_size);

  int n_local = faces.size();
  std::vector<int> sizes(comm_size);
  MPI_Allgather(&n_local, 1, MPI_INT, sizes.data(), 1, MPI_INT, comm);
  std::vector<int> offset(comm_size);
  std::exclusive_scan(sizes.begin(), sizes.end(), offset.begin(), 0);
  const int total = std::accumulate(sizes.begin(), sizes.end(), 0);

  std::vector<double> all(total);
  MPI_Allgatherv(
      faces.data(), n_local, MPI_DOUBLE, all.data(), sizes.data(), offset.data(), MPI_DOUBLE, comm);
  faces.swap(all);
}
}

mfem::Vector
MFEMPMLStretch::meshBarycenter(mfem::ParMesh & mesh, MPI_Comm comm) const
{
  mfem::Vector weighted_sum(_dim);
  weighted_sum = 0.0;
  double volume = 0.0;

  for (const auto e : make_range(mesh.GetNE()))
  {
    const double element_volume = mesh.GetElementVolume(e);
    mfem::Vector element_center(3);
    mesh.GetElementCenter(e, element_center);
    for (const auto d : make_range(_dim))
      weighted_sum[d] += element_volume * element_center[d];
    volume += element_volume;
  }

  MPI_Allreduce(MPI_IN_PLACE, weighted_sum.GetData(), _dim, MFEM_MPI_REAL_T, MPI_SUM, comm);
  MPI_Allreduce(MPI_IN_PLACE, &volume, 1, MPI_DOUBLE, MPI_SUM, comm);
  if (volume <= 0.0)
    mooseError("MFEMPMLStretch: the mesh has no volume, so its barycenter cannot be computed.");
  weighted_sum /= volume;
  return weighted_sum;
}

MFEMPMLStretch::MFEMPMLStretch(mfem::ParMesh & mesh,
                               const mfem::Array<int> & pml_attributes,
                               const mfem::Vector & reference_point,
                               double decay_coefficient,
                               double decay_polynomial,
                               MPI_Comm comm)
  : _dim(mesh.Dimension()),
    _reference_point(reference_point),
    _decay_coefficient(decay_coefficient),
    _decay_polynomial(decay_polynomial)
{
  // No reference point supplied, so measure depth from the barycenter of the mesh.
  if (_reference_point.Size() == 0)
    _reference_point = meshBarycenter(mesh, comm);

  mesh.ExchangeFaceNbrData();

  auto in_pml = [&pml_attributes](int attribute)
  { return pml_attributes.Find(attribute) != -1; };

  // Depth is measured along rays leaving the reference point, which is only meaningful if that
  // point lies inside the mesh but outside the layer: a ray must cross the inner surface of the
  // layer before the outer one. Locate the element containing it to confirm both conditions.
  {
    mfem::DenseMatrix point_matrix(_dim, 1);
    for (const auto d : make_range(_dim))
      point_matrix(d, 0) = _reference_point[d];
    mfem::Array<int> element_ids;
    mfem::Array<mfem::IntegrationPoint> integration_points;
    mesh.FindPoints(point_matrix, element_ids, integration_points, false);

    // 0: not on this rank, 1: found outside the layer, 2: found inside the layer. Taking the
    // maximum across ranks reports the point as being in the layer if any rank places it there.
    int location = 0;
    if (element_ids[0] >= 0)
      location = in_pml(mesh.GetAttribute(element_ids[0])) ? 2 : 1;
    MPI_Allreduce(MPI_IN_PLACE, &location, 1, MPI_INT, MPI_MAX, comm);

    if (location == 0)
      mooseError("MFEMPMLStretch: the reference point lies outside the mesh. Depth into the "
                 "perfectly matched layer is measured along rays leaving this point, so it must "
                 "lie inside the mesh.");
    if (location == 2)
      mooseError("MFEMPMLStretch: the reference point lies inside the perfectly matched layer. It "
                 "must lie in the interior of the domain, outside the layer, so that a ray leaving "
                 "it crosses the inner surface of the layer before the outer one.");
  }

  // Append a face to a surface as a flat list of simplex coordinates: segments in 2D, triangles in
  // 3D. Curved faces are represented by their straight sided vertex geometry.
  auto append_face = [&](std::vector<double> & surface, int face)
  {
    auto push_point = [&](const double * point)
    {
      for (const auto d : make_range(_dim))
        surface.push_back(point[d]);
    };

    mfem::Array<int> face_vertices;
    mesh.GetFaceVertices(face, face_vertices);
    const int n_vertices = face_vertices.Size();

    // A face with as many vertices as there are spatial dimensions is already a simplex.
    if (n_vertices == _dim)
    {
      for (const auto k : make_range(n_vertices))
        push_point(mesh.GetVertex(face_vertices[k]));
      return;
    }

    // Any larger face, such as the quadrilaterals of hexahedral, wedge and pyramid meshes, is split
    // about its centroid. This approximates a warped face symmetrically instead of biasing it along
    // one diagonal.
    mfem::Vector centroid(_dim);
    centroid = 0.0;
    for (const auto k : make_range(n_vertices))
      for (const auto d : make_range(_dim))
        centroid[d] += mesh.GetVertex(face_vertices[k])[d] / n_vertices;

    for (const auto k : make_range(n_vertices))
    {
      push_point(mesh.GetVertex(face_vertices[k]));
      push_point(mesh.GetVertex(face_vertices[(k + 1) % n_vertices]));
      push_point(centroid.GetData());
    }
  };

  for (const auto f : make_range(mesh.GetNumFaces()))
  {
    const auto info = mesh.GetFaceInformation(f);
    int elem1 = -1, elem2 = -1;
    mesh.GetFaceElements(f, &elem1, &elem2);
    const bool elem1_in_pml = (elem1 >= 0) && in_pml(mesh.GetAttribute(elem1));

    if (info.IsBoundary())
    {
      // Exterior face: part of the outer surface when its element is in the layer.
      if (elem1_in_pml)
        append_face(_outer_faces, f);
      continue;
    }

    bool elem2_in_pml = false;
    if (info.IsShared())
    {
      // The second element lives on another rank, so its attribute is read through the face
      // neighbour transformation. Shared inner faces are contributed by both ranks; the resulting
      // duplicates are harmless because only the nearest intersection is used.
      mfem::ElementTransformation * transformation =
          mesh.GetFaceNbrElementTransformation(info.element[1].index);
      elem2_in_pml = in_pml(transformation->Attribute);
    }
    else
      elem2_in_pml = (elem2 >= 0) && in_pml(mesh.GetAttribute(elem2));

    // Inner surface: exactly one side of the face is in the layer.
    if (elem1_in_pml != elem2_in_pml)
      append_face(_inner_faces, f);
  }

  replicateFaces(_inner_faces, comm);
  replicateFaces(_outer_faces, comm);

  if (_inner_faces.empty() || _outer_faces.empty())
    mooseError("MFEMPMLStretch: the perfectly matched layer has no inner or no outer surface. "
               "Check that the kernel is restricted to a layer of elements that borders both the "
               "rest of the domain and the exterior of the mesh.");
}

double
MFEMPMLStretch::castRay(const mfem::Vector & direction, const std::vector<double> & faces) const
{
  constexpr double eps = 1e-12;
  double nearest = -1.0;

  if (_dim == 2)
  {
    // Ray p0 + t d against the segment a + u (b - a), solved by Cramer's rule for t and u. The
    // crossing counts when it lies ahead of the reference point and within the segment.
    for (std::size_t i = 0; i + 4 <= faces.size(); i += 4)
    {
      const double ax = faces[i], ay = faces[i + 1];
      const double bx = faces[i + 2], by = faces[i + 3];
      const double ex = bx - ax, ey = by - ay;
      const double determinant = direction[1] * ex - direction[0] * ey;
      // A vanishing determinant means the ray is parallel to the segment.
      if (std::abs(determinant) < eps)
        continue;
      const double rx = ax - _reference_point[0], ry = ay - _reference_point[1];
      const double t = (ry * ex - rx * ey) / determinant;
      const double u = (ry * direction[0] - rx * direction[1]) / determinant;
      if (t > eps && u >= -eps && u <= 1.0 + eps && (nearest < 0.0 || t < nearest))
        nearest = t;
    }
  }
  else
  {
    // Moller-Trumbore ray/triangle intersection: Cramer's rule on the system that equates the ray
    // with the barycentric parametrisation of the triangle, so the containment test in u and v
    // falls out of the same determinants as the distance t.
    for (std::size_t i = 0; i + 9 <= faces.size(); i += 9)
    {
      const double * a = &faces[i];
      const double * b = &faces[i + 3];
      const double * c = &faces[i + 6];
      double edge1[3], edge2[3], pvec[3], tvec[3], qvec[3];
      for (const auto d : make_range(3))
      {
        edge1[d] = b[d] - a[d];
        edge2[d] = c[d] - a[d];
        tvec[d] = _reference_point[d] - a[d];
      }
      pvec[0] = direction[1] * edge2[2] - direction[2] * edge2[1];
      pvec[1] = direction[2] * edge2[0] - direction[0] * edge2[2];
      pvec[2] = direction[0] * edge2[1] - direction[1] * edge2[0];
      const double determinant = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];
      // A vanishing determinant means the ray is parallel to the plane of the triangle.
      if (std::abs(determinant) < eps)
        continue;
      const double inverse_determinant = 1.0 / determinant;
      const double u =
          (tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2]) * inverse_determinant;
      if (u < -eps || u > 1.0 + eps)
        continue;
      qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
      qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
      qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];
      const double v =
          (direction[0] * qvec[0] + direction[1] * qvec[1] + direction[2] * qvec[2]) *
          inverse_determinant;
      if (v < -eps || u + v > 1.0 + eps)
        continue;
      const double t =
          (edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2]) * inverse_determinant;
      if (t > eps && (nearest < 0.0 || t < nearest))
        nearest = t;
    }
  }
  return nearest;
}

void
MFEMPMLStretch::evaluate(const mfem::Vector & x,
                         mfem::Vector & radial_direction,
                         std::complex<double> & radial_factor,
                         std::complex<double> & tangential_factor) const
{
  constexpr std::complex<double> imaginary_unit(0.0, 1.0);

  // The direction is normalised so that the ray parameter returned by castRay is a distance.
  radial_direction.SetSize(_dim);
  double radius = 0.0;
  for (const auto d : make_range(_dim))
  {
    radial_direction[d] = x[d] - _reference_point[d];
    radius += radial_direction[d] * radial_direction[d];
  }
  radius = std::sqrt(radius);

  if (radius <= 0.0)
    mooseError("MFEMPMLStretch: an evaluation point coincides with the reference point, so the "
               "radial direction is undefined there.");

  radial_factor = 1.0;
  tangential_factor = 1.0;
  radial_direction /= radius;

  const double inner_radius = castRay(radial_direction, _inner_faces);
  const double outer_radius = castRay(radial_direction, _outer_faces);
  if (inner_radius < 0.0 || outer_radius < 0.0)
    mooseError("MFEMPMLStretch: a ray from the reference point missed the inner or the outer "
               "surface of the perfectly matched layer. The reference point must lie inside the "
               "mesh and both surfaces must be star shaped about it.");

  const double thickness = outer_radius - inner_radius;
  const double depth = radius - inner_radius;
  if (thickness <= 0.0 || depth <= 0.0)
    return; // Outside the layer, so the coordinate is left unstretched.

  const double n = _decay_polynomial;
  const double scale = _decay_coefficient / std::pow(thickness, n);
  radial_factor = 1.0 + imaginary_unit * n * scale * std::pow(depth, n - 1.0);
  tangential_factor = 1.0 + imaginary_unit * scale * std::pow(depth, n) / radius;
}

#endif
