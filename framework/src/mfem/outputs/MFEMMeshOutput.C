//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeshOutput.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

registerMooseObject("MooseApp", MFEMMeshOutput);

namespace
{
/**
 * Rotate the vertices of a tetrahedron into the canonical order v0 < v1 < min(v2, v3). Only
 * even permutations are used, so the orientation of the element is preserved.
 */
void
canonicalizeTetVertices(int * v)
{
  // Move the smallest vertex to the front. Swapping the remaining pair at the same time makes
  // this a double transposition, which is an even permutation.
  const auto i_min = std::min_element(v, v + 4) - v;
  if (i_min != 0)
  {
    std::swap(v[0], v[i_min]);
    std::swap(v[i_min % 3 + 1], v[(i_min + 1) % 3 + 1]);
  }
  // A cyclic shift of the last three vertices is also an even permutation.
  std::rotate(v + 1, std::min_element(v + 1, v + 4), v + 4);
}

/**
 * Return the local index of the degree of freedom of \p old_fe that sits at the same physical
 * point as local degree of freedom \p new_dof of \p new_fe, where the two tetrahedra have the
 * same vertices listed in the orders \p old_vertices and \p new_vertices.
 */
int
matchingTetDof(const mfem::FiniteElement & old_fe,
               const mfem::FiniteElement & new_fe,
               const int * old_vertices,
               const int * new_vertices,
               const int new_dof)
{
  const auto & new_point = new_fe.GetNodes().IntPoint(new_dof);
  const mfem::real_t new_barycentric[4] = {
      1. - new_point.x - new_point.y - new_point.z, new_point.x, new_point.y, new_point.z};

  // Barycentric coordinates follow their vertex when the vertices are permuted.
  mfem::real_t old_barycentric[4];
  for (const auto k : make_range(4))
    old_barycentric[std::find(old_vertices, old_vertices + 4, new_vertices[k]) - old_vertices] =
        new_barycentric[k];

  // The nodal points of a tetrahedral element are symmetric under vertex permutations, so the
  // mapped point coincides with an old nodal point up to round-off. Take the closest one.
  const auto & old_nodes = old_fe.GetNodes();
  int closest = 0;
  mfem::real_t closest_distance = std::numeric_limits<mfem::real_t>::max();
  for (const auto j : make_range(old_nodes.GetNPoints()))
  {
    const auto & old_point = old_nodes.IntPoint(j);
    const auto distance = std::abs(old_point.x - old_barycentric[1]) +
                          std::abs(old_point.y - old_barycentric[2]) +
                          std::abs(old_point.z - old_barycentric[3]);
    if (distance < closest_distance)
    {
      closest = j;
      closest_distance = distance;
    }
  }
  return closest;
}

/**
 * Return a copy of \p elem belonging to \p mesh, with its vertices renumbered by
 * \p new_vertex_ids. Tetrahedra, and the triangles bounding a 3D mesh, are also rotated into
 * their canonical vertex order.
 */
mfem::Element *
renumberedElement(const mfem::Element & elem,
                  const std::vector<int> & new_vertex_ids,
                  mfem::Mesh & mesh)
{
  auto * const copy = elem.Duplicate(&mesh);
  auto * const v = copy->GetVertices();
  for (const auto k : make_range(copy->GetNVertices()))
    v[k] = new_vertex_ids[v[k]];

  if (copy->GetType() == mfem::Element::TETRAHEDRON)
    canonicalizeTetVertices(v);
  else if (copy->GetType() == mfem::Element::TRIANGLE && mesh.Dimension() == 3)
    std::rotate(v, std::min_element(v, v + 3), v + 3);
  return copy;
}

/**
 * Return the indices of \p elements sorted by vertex list and then by attribute. This order
 * depends only on the elements themselves, not on the order they were given in.
 */
std::vector<int>
canonicalElementOrder(const std::vector<mfem::Element *> & elements)
{
  std::vector<int> order(elements.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(),
            order.end(),
            [&elements](const int i, const int j)
            {
              const auto & a = *elements[i];
              const auto & b = *elements[j];
              const auto * const va = a.GetVertices();
              const auto * const vb = b.GetVertices();
              const auto na = a.GetNVertices();
              const auto nb = b.GetNVertices();
              if (!std::equal(va, va + na, vb, vb + nb))
                return std::lexicographical_compare(va, va + na, vb, vb + nb);
              return a.GetAttribute() < b.GetAttribute();
            });
  return order;
}

/**
 * Return the coordinates of the vertices of \p mesh, stored vertex by vertex. The vertex
 * coordinates of a curved mesh gathered by mfem::ParMesh::GetSerialMesh are not set, so for
 * curved meshes they are read from the nodes instead.
 */
std::vector<mfem::real_t>
vertexCoordinates(const mfem::Mesh & mesh)
{
  const auto sdim = mesh.SpaceDimension();
  std::vector<mfem::real_t> coords(mesh.GetNV() * sdim);
  const auto * const nodes = mesh.GetNodes();
  if (!nodes)
    for (const auto v : make_range(mesh.GetNV()))
      std::copy_n(mesh.GetVertex(v), sdim, coords.begin() + v * sdim);
  else if (nodes->FESpace()->GetNVDofs() > 0)
  {
    // Continuous nodes hold the exact vertex positions in their vertex degrees of freedom.
    mfem::Array<int> vdofs;
    for (const auto v : make_range(mesh.GetNV()))
    {
      nodes->FESpace()->GetVertexVDofs(v, vdofs);
      for (const auto c : make_range(sdim))
        coords[v * sdim + c] = (*nodes)(vdofs[c]);
    }
  }
  else
  {
    // Discontinuous nodes have no vertex degrees of freedom, so evaluate them at the vertices.
    mfem::Vector values;
    for (const auto c : make_range(sdim))
    {
      nodes->GetNodalValues(values, c + 1);
      for (const auto v : make_range(mesh.GetNV()))
        coords[v * sdim + c] = values(v);
    }
  }
  return coords;
}

/**
 * Put a conforming mesh into a canonical form, so that meshes with the same vertices and elements
 * are written out identically however they were numbered, ordered or partitioned. Vertices are
 * numbered in lexicographic order of their coordinates, tetrahedra and boundary triangles are
 * rotated into the vertex order of the deprecated mfem::Mesh::ReorientTetMesh, and elements and
 * boundary elements are sorted by their vertex lists. The mesh is rebuilt so that its
 * connectivity tables and, for curved meshes, its nodes stay consistent.
 */
void
canonicalizeMesh(mfem::Mesh & mesh)
{
  // Nonconforming meshes are written from their mfem::NCMesh, which this does not change.
  if (mesh.GetNE() == 0 || mesh.Nonconforming() || mesh.NURBSext)
    return;

  const auto sdim = mesh.SpaceDimension();
  const auto coords = vertexCoordinates(mesh);
  std::vector<int> vertex_order(mesh.GetNV());
  std::iota(vertex_order.begin(), vertex_order.end(), 0);
  std::sort(vertex_order.begin(),
            vertex_order.end(),
            [&coords, sdim](const int i, const int j)
            {
              const auto * const xi = coords.data() + i * sdim;
              const auto * const xj = coords.data() + j * sdim;
              return std::lexicographical_compare(xi, xi + sdim, xj, xj + sdim);
            });
  std::vector<int> new_vertex_ids(mesh.GetNV());
  for (const auto i : index_range(vertex_order))
    new_vertex_ids[vertex_order[i]] = i;

  mfem::Mesh canonical(mesh.Dimension(), mesh.GetNV(), mesh.GetNE(), mesh.GetNBE(), sdim);
  for (const auto i : vertex_order)
    canonical.AddVertex(coords.data() + i * sdim);

  std::vector<mfem::Element *> elements(mesh.GetNE());
  for (const auto i : make_range(mesh.GetNE()))
    elements[i] = renumberedElement(*mesh.GetElement(i), new_vertex_ids, canonical);
  const auto element_order = canonicalElementOrder(elements);
  for (const auto i : element_order)
    canonical.AddElement(elements[i]);

  std::vector<mfem::Element *> bdr_elements(mesh.GetNBE());
  for (const auto i : make_range(mesh.GetNBE()))
    bdr_elements[i] = renumberedElement(*mesh.GetBdrElement(i), new_vertex_ids, canonical);
  for (const auto i : canonicalElementOrder(bdr_elements))
    canonical.AddBdrElement(bdr_elements[i]);

  canonical.FinalizeTopology(false);
  canonical.Finalize();
  canonical.attribute_sets.attr_sets = mesh.attribute_sets.attr_sets;
  canonical.bdr_attribute_sets.attr_sets = mesh.bdr_attribute_sets.attr_sets;

  if (const auto * const nodes = mesh.GetNodes())
  {
    const auto & fes = *nodes->FESpace();
    auto * const fec = mfem::FiniteElementCollection::New(fes.FEColl()->Name());
    auto * const canonical_nodes = new mfem::GridFunction(
        new mfem::FiniteElementSpace(&canonical, fec, fes.GetVDim(), fes.GetOrdering()));
    canonical_nodes->MakeOwner(fec);
    const auto & canonical_fes = *canonical_nodes->FESpace();

    // Copy each element's nodal values, permuting the degrees of freedom of the rotated
    // tetrahedra. Element vector dofs are grouped by component.
    mfem::Array<int> vdofs, canonical_vdofs;
    mfem::Vector values, canonical_values;
    for (const auto i : index_range(element_order))
    {
      const auto old_i = element_order[i];
      fes.GetElementVDofs(old_i, vdofs);
      canonical_fes.GetElementVDofs(i, canonical_vdofs);
      nodes->GetSubVector(vdofs, values);
      canonical_values.SetSize(values.Size());

      const auto & fe = *fes.GetFE(old_i);
      const auto & canonical_fe = *canonical_fes.GetFE(i);
      const bool is_tet = mesh.GetElementType(old_i) == mfem::Element::TETRAHEDRON;
      int renumbered_vertices[4];
      if (is_tet)
        for (const auto k : make_range(4))
          renumbered_vertices[k] = new_vertex_ids[mesh.GetElement(old_i)->GetVertices()[k]];

      const int ndofs = canonical_fe.GetDof();
      for (const auto a : make_range(ndofs))
      {
        const int b = is_tet ? matchingTetDof(fe,
                                              canonical_fe,
                                              renumbered_vertices,
                                              canonical.GetElement(i)->GetVertices(),
                                              a)
                             : a;
        for (const auto c : make_range(fes.GetVDim()))
          canonical_values(c * ndofs + a) = values(c * ndofs + b);
      }
      canonical_nodes->SetSubVector(canonical_vdofs, canonical_values);
    }
    canonical.NewNodes(*canonical_nodes, true);
  }

  mesh = std::move(canonical);
}
}

InputParameters
MFEMMeshOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Writes the mesh of an MFEMProblem, or one of its submeshes, in the "
                             "native MFEM mesh format.");
  params.addParam<std::string>("submesh",
                               "Submesh to output variables on. Leave blank to use base mesh.");
  MooseEnum ordering("NONE HILBERT GECKO", "NONE", false);
  params.addParam<MooseEnum>(
      "ordering",
      ordering,
      "Whether to reorder the elements of the mesh. Options are NONE to do nothing, HILBERT to "
      "perform a spatial sort on the elements so they approximately follow the Hilbert curve, and "
      "GECKO to use the Gecko library to order elements for increased memory coherence.");
  params.addParam<int>("precision", 16, "Number of digits to use with ASCII output.");
  return params;
}

MFEMMeshOutput::MFEMMeshOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _pmesh(parameters.isParamValid("submesh")
               ? cast_ptr<MFEMProblem *>(_problem_ptr)
                     ->getProblemData()
                     .submeshes.GetRef(getParam<std::string>("submesh"))
               : cast_ptr<MFEMProblem *>(_problem_ptr)->mfemParMesh()),
    _ordering(getParam<MooseEnum>("ordering")),
    _precision(getParam<int>("precision"))
{
}

std::string
MFEMMeshOutput::filename()
{
  std::ostringstream output;
  output << _file_base << ".mesh";

  // Add the _000x extension to the file
  if (_file_num > 1)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
MFEMMeshOutput::output()
{
  constexpr int save_rank = 0;
  mfem::Mesh serial_mesh = _pmesh.GetSerialMesh(save_rank);
  // Reordering a canonical mesh is deterministic, so the output does not depend on how the mesh
  // was numbered or partitioned.
  canonicalizeMesh(serial_mesh);

  if (_ordering > 0)
  {
    mfem::Array<int> new_order;
    if (_ordering == 1)
      serial_mesh.GetHilbertElementOrdering(new_order);
    else
      // FIXME: Add support for various Gecko element ordering configs
      serial_mesh.GetGeckoElementOrdering(new_order);
    serial_mesh.ReorderElements(new_order);
  }

  if (processor_id() == save_rank)
  {
    serial_mesh.Save(filename().c_str(), _precision);
  }
}

#endif
