//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTopology.h"

std::vector<mfem::Vector>
ConvertVectorOfVectorsToMFEM(std::vector<std::vector<mfem::real_t>> vector_of_vectors)
{
  std::vector<mfem::Vector> mfem_vectors;
  for (auto & vector : vector_of_vectors)
  {
    const mfem::Vector mfem_vector(&vector[0], vector.size());
    mfem_vectors.push_back(mfem_vector);
  }
  return mfem_vectors;
}

InputParameters
MFEMTopology::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::vector<mfem::real_t>>>(
      "lattice_vectors", {}, "Translation vectors matching pairs of equivalent vertices.");
  params.addParam<unsigned int>(
      "rotational_symmetry_order",
      1,
      "Order of rotational symmetry around z axis. Number of whole copies of mesh in 2 pi rotation "
      "about axis of rotational symmetry.");
  return params;
}

MFEMTopology::MFEMTopology(const InputParameters & parameters)
  : _lattice_vectors(ConvertVectorOfVectorsToMFEM(
        parameters.get<std::vector<std::vector<mfem::real_t>>>("lattice_vectors"))),
    _rotational_symmetry_order(parameters.get<unsigned int>("rotational_symmetry_order"))
{
  for (const auto & lattice_vector : _lattice_vectors)
    DeclareTranslationalSymmetry(lattice_vector);

  if (_rotational_symmetry_order > 1)
    DeclareRotationalSymmetry(_rotational_symmetry_order);
}

void
MFEMTopology::DeclareTranslationalSymmetry(const mfem::Vector & translation)
{
  _periodic = true;
  auto translational_symmetry = std::make_shared<TranslationalSymmetry>(translation);
  _symmetry_transforms.push_back(translational_symmetry);
}

void
MFEMTopology::DeclareRotationalSymmetry(const unsigned int rotational_symmetry_order)
{
  _periodic = true;
  auto rotational_symmetry = std::make_shared<RotationalSymmetry>(rotational_symmetry_order);
  _symmetry_transforms.push_back(rotational_symmetry);
}

std::vector<int>
MFEMTopology::CreateTopologicallyEquivalentVertexMap(const mfem::Mesh & mesh) const
{
  const int sdim = mesh.SpaceDimension();

  mfem::Vector coord(sdim), at(sdim), dx(sdim);
  mfem::Vector xMax(sdim), xMin(sdim), xDiff(sdim);
  xMax = xMin = xDiff = 0.0;

  // Get a list of all vertices on the boundary
  std::unordered_set<int> bdr_v;
  for (int be = 0; be < mesh.GetNBE(); be++)
  {
    mfem::Array<int> dofs;
    mesh.GetBdrElementVertices(be, dofs);

    for (int i = 0; i < dofs.Size(); i++)
    {
      bdr_v.insert(dofs[i]);

      coord = mesh.GetVertex(dofs[i]);
      for (int j = 0; j < sdim; j++)
      {
        xMax[j] = std::max(xMax[j], coord[j]);
        xMin[j] = std::min(xMin[j], coord[j]);
      }
    }
  }
  add(xMax, -1.0, xMin, xDiff);
  mfem::real_t dia = xDiff.Norml2(); // compute mesh diameter

  // We now identify coincident vertices. Several originally distinct vertices
  // may become coincident under the periodic mapping. One of these vertices
  // will be identified as the "primary" vertex, and all other coincident
  // vertices will be considered as "replicas".

  // replica2primary[v] is the index of the primary vertex of replica `v`
  std::unordered_map<int, int> replica2primary;
  // primary2replicas[v] is a set of indices of replicas of primary vertex `v`
  std::unordered_map<int, std::unordered_set<int>> primary2replicas;

  // Create a KD-tree containing all the boundary vertices
  std::unique_ptr<mfem::KDTreeBase<int, mfem::real_t>> kdtree;
  if (sdim == 1)
  {
    kdtree.reset(new mfem::KDTree1D);
  }
  else if (sdim == 2)
  {
    kdtree.reset(new mfem::KDTree2D);
  }
  else if (sdim == 3)
  {
    kdtree.reset(new mfem::KDTree3D);
  }
  else
  {
    MFEM_ABORT("Invalid space dimension.");
  }

  // We begin with the assumption that all vertices are primary, and that there
  // are no replicas.
  for (const int v : bdr_v)
  {
    primary2replicas[v];
    kdtree->AddPoint(mesh.GetVertex(v), v);
  }

  kdtree->Sort();

  // Make `r` and all of `r`'s replicas be replicas of `p`. Delete `r` from the
  // list of primary vertices.
  auto make_replica = [&replica2primary, &primary2replicas](int r, int p)
  {
    if (r == p)
      return;

    primary2replicas[p].insert(r);
    replica2primary[r] = p;
    for (const int s : primary2replicas[r])
    {
      primary2replicas[p].insert(s);
      replica2primary[s] = p;
    }
    primary2replicas.erase(r);
  };

  // Iterate over all mesh symmetries
  for (const auto & symmetry_transform : _symmetry_transforms)
  {
    for (int vi : bdr_v)
    {
      coord = mesh.GetVertex(vi);
      symmetry_transform->ApplyTransform(coord, at);
      const int vj = kdtree->FindClosestPoint(at.GetData());
      coord = mesh.GetVertex(vj);
      add(at, -1.0, coord, dx);
      mfem::real_t tol = 1e-8;
      if (dx.Norml2() > dia * tol)
        continue;

      // The two vertices vi and vj are coincident.
      // Are vertices `vi` and `vj` already primary?
      const bool pi = primary2replicas.find(vi) != primary2replicas.end();
      const bool pj = primary2replicas.find(vj) != primary2replicas.end();

      if (pi && pj)
      {
        // Both vertices are currently primary
        // Demote `vj` to be a replica of `vi`
        make_replica(vj, vi);
      }
      else if (pi && !pj)
      {
        // `vi` is primary and `vj` is a replica
        const int owner_of_vj = replica2primary[vj];
        // Make `vi` and its replicas be replicas of `vj`'s owner
        make_replica(vi, owner_of_vj);
      }
      else if (!pi && pj)
      {
        // `vi` is currently a replica and `vj` is currently primary
        // Make `vj` and its replicas be replicas of `vi`'s owner
        const int owner_of_vi = replica2primary[vi];
        make_replica(vj, owner_of_vi);
      }
      else
      {
        // Both vertices are currently replicas
        // Make `vj`'s owner and all of its owner's replicas be replicas
        // of `vi`'s owner
        const int owner_of_vi = replica2primary[vi];
        const int owner_of_vj = replica2primary[vj];
        make_replica(owner_of_vj, owner_of_vi);
      }
    }
  }

  std::vector<int> v2v(mesh.GetNV());
  for (size_t i = 0; i < v2v.size(); i++)
    v2v[i] = static_cast<int>(i);
  for (const auto & r2p : replica2primary)
    v2v[r2p.first] = r2p.second;

  return v2v;
}

#endif
