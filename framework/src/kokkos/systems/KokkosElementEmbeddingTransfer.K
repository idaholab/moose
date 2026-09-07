//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementEmbeddingTransfer.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dof_object.h"
#include "libmesh/enum_to_string.h"

namespace Moose::Kokkos
{

ElementEmbedding::ElementEmbedding(const DofSpace & coarse,
                                   const DofSpace & fine,
                                   const Array3D<Array2D<Real>> & embedding,
                                   const Array<Vector> & vectors)
  : _coarse(coarse), _fine(fine), _embedding(embedding), _vectors(vectors)
{
  // The two sides share the mesh and the variables, so both directions run over the same
  // (element, variable) pairs
  _thread.resize(_fine.kokkosMesh().getNumLocalElements(), _fine.getNumVars());
}

void
ElementEmbedding::prolong()
{
  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, ProlongLoop> policy(
      0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

void ElementEmbedding::restrict()
{
  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, RestrictLoop> policy(
      0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

void
ElementEmbedding::multiplicity()
{
  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>, MultiplicityLoop> policy(
      0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION const Array2D<Real> &
ElementEmbedding::table(const ContiguousElementID elem, const unsigned int var) const
{
  const auto info = _fine.kokkosMesh().getElementInfo(elem);

  return _embedding(info.subdomain, info.type, var);
}

KOKKOS_FUNCTION void
ElementEmbedding::operator()(ProlongLoop, const ThreadID tid) const
{
  const auto elem = _thread(tid, 0);
  const auto var = _thread(tid, 1);

  const auto & embedding = table(elem, var);

  if (!embedding.isAlloc())
    return;

  const unsigned int num_fine = embedding.n(0);
  const unsigned int num_coarse = embedding.n(1);

  for (unsigned int i = 0; i < num_fine; ++i)
  {
    Real value = 0;

    for (unsigned int j = 0; j < num_coarse; ++j)
      value +=
          embedding(i, j) * _vectors[EMBEDDING_COARSE][_coarse.getElemLocalDofIndex(elem, j, var)];

    ::Kokkos::atomic_add(&_vectors[EMBEDDING_FINE][_fine.getElemLocalDofIndex(elem, i, var)],
                         value);
  }
}

KOKKOS_FUNCTION void
ElementEmbedding::operator()(RestrictLoop, const ThreadID tid) const
{
  const auto elem = _thread(tid, 0);
  const auto var = _thread(tid, 1);

  const auto & embedding = table(elem, var);

  if (!embedding.isAlloc())
    return;

  const unsigned int num_fine = embedding.n(0);
  const unsigned int num_coarse = embedding.n(1);

  for (unsigned int j = 0; j < num_coarse; ++j)
  {
    Real value = 0;

    for (unsigned int i = 0; i < num_fine; ++i)
      value += embedding(i, j) * _vectors[EMBEDDING_FINE][_fine.getElemLocalDofIndex(elem, i, var)];

    ::Kokkos::atomic_add(&_vectors[EMBEDDING_COARSE][_coarse.getElemLocalDofIndex(elem, j, var)],
                         value);
  }
}

KOKKOS_FUNCTION void
ElementEmbedding::operator()(MultiplicityLoop, const ThreadID tid) const
{
  const auto elem = _thread(tid, 0);
  const auto var = _thread(tid, 1);

  const auto & embedding = table(elem, var);

  if (!embedding.isAlloc())
    return;

  const unsigned int num_fine = embedding.n(0);

  for (unsigned int i = 0; i < num_fine; ++i)
    ::Kokkos::atomic_add(&_vectors[EMBEDDING_FINE][_fine.getElemLocalDofIndex(elem, i, var)], 1.0);
}

ElementEmbeddingTransfer::ElementEmbeddingTransfer(const Assembly & assembly,
                                                   const TransferSide & coarse,
                                                   const TransferSide & fine)
  : LevelTransfer(coarse, fine),
    _assembly(assembly),
    // The loops gather an input by local, ghost-inclusive DOF index, so each direction's input
    // arrives through a ghosted vector of the side it comes from. A level in the middle of a
    // hierarchy is the coarse side of one pair and the fine side of another, which the two names
    // keep apart.
    _coarse_input(_coarse.sys.add_vector("prolongation input", false, libMesh::GHOSTED)),
    _fine_input(_fine.sys.add_vector("restriction input", false, libMesh::GHOSTED)),
    _inverse_multiplicity(_fine.sys.add_vector("inverse element multiplicity", false))
{
  _vectors.create(EMBEDDING_NUM_SLOTS);

  buildEmbedding();
  buildMultiplicity();
}

void
ElementEmbeddingTransfer::buildEmbedding()
{
  const auto & mesh = _assembly.kokkosMesh();
  const auto num_vars = _coarse.dof_space.getNumVars();

  _embedding.create(mesh.getNumSubdomains(), mesh.getNumLocalElementTypes(), num_vars);

  for (const auto sid : make_range(mesh.getNumSubdomains()))
    for (const auto & [elem_type, elem_type_id] : mesh.getElementTypeMap())
      for (const auto var : make_range(num_vars))
      {
        const auto coarse_fe = _coarse.fe_types[var];
        const auto fine_fe = _fine.fe_types[var];

        const auto num_coarse = _assembly.getNumDofs(elem_type_id, coarse_fe);
        const auto num_fine = _assembly.getNumDofs(elem_type_id, fine_fe);

        if (!num_coarse || !num_fine)
          continue;

        const auto & coarse_phi = _assembly.getPhi(sid, elem_type_id, coarse_fe);
        const auto & coarse_grad_phi = _assembly.getGradPhi(sid, elem_type_id, coarse_fe);
        const auto & fine_phi = _assembly.getPhi(sid, elem_type_id, fine_fe);
        const auto & fine_grad_phi = _assembly.getGradPhi(sid, elem_type_id, fine_fe);

        const auto num_qps = fine_phi.n(1);

        // Each coarse basis function is fit in the fine basis in the least-squares sense against
        // both the values and the reference gradients at the shared quadrature points. The fit is
        // exact whenever the coarse space is contained in the fine space, which is the case the
        // transfer is for, and the normal equations are the same tables the level operators
        // contract, so the embedding is consistent with them by construction.
        libMesh::DenseMatrix<Real> normal(num_fine, num_fine);
        libMesh::DenseMatrix<Real> rhs(num_fine, num_coarse);

        for (const auto i : make_range(num_fine))
        {
          for (const auto j : make_range(num_fine))
          {
            Real entry = 0;

            for (const auto qp : make_range(num_qps))
              entry +=
                  fine_phi(i, qp) * fine_phi(j, qp) + fine_grad_phi(i, qp) * fine_grad_phi(j, qp);

            normal(i, j) = entry;
          }

          for (const auto j : make_range(num_coarse))
          {
            Real entry = 0;

            for (const auto qp : make_range(num_qps))
              entry += fine_phi(i, qp) * coarse_phi(j, qp) +
                       fine_grad_phi(i, qp) * coarse_grad_phi(j, qp);

            rhs(i, j) = entry;
          }
        }

        auto & embedding = _embedding(sid, elem_type_id, var);

        embedding.create(num_fine, num_coarse);

        libMesh::DenseVector<Real> column(num_fine);
        libMesh::DenseVector<Real> solution;

        for (const auto j : make_range(num_coarse))
        {
          for (const auto i : make_range(num_fine))
            column(i) = rhs(i, j);

          // The normal matrix is a Gram matrix of the fine basis, so it is symmetric positive
          // definite; the factorization is formed on the first column and reused for the rest
          normal.cholesky_solve(column, solution);

          for (const auto i : make_range(num_fine))
            embedding(i, j) = solution(i);
        }

        Real error = 0;
        Real scale = 0;

        for (const auto j : make_range(num_coarse))
          for (const auto qp : make_range(num_qps))
          {
            Real value = 0;
            Real3 grad = 0;

            for (const auto i : make_range(num_fine))
            {
              value += embedding(i, j) * fine_phi(i, qp);
              grad += embedding(i, j) * fine_grad_phi(i, qp);
            }

            error = std::max(error, std::abs(value - coarse_phi(j, qp)));
            scale = std::max(scale, std::abs(coarse_phi(j, qp)));

            for (const auto d : make_range(3u))
            {
              error = std::max(error, std::abs(grad(d) - coarse_grad_phi(j, qp)(d)));
              scale = std::max(scale, std::abs(coarse_grad_phi(j, qp)(d)));
            }
          }

        // An exact fit leaves the residual of a small dense symmetric solve, so the margin over the
        // unit roundoff of double precision covers the conditioning of the normal equations
        const auto tolerance = 1e-9 * std::max(scale, 1.0);

        if (error > tolerance)
          mooseError(
              "The order ",
              _coarse.sys.variable(var).type().order.get_order(),
              " space of variable '",
              _coarse.sys.variable(var).name(),
              "' is not contained in its order ",
              _fine.sys.variable(var).type().order.get_order(),
              " space on ",
              libMesh::Utility::enum_to_string(elem_type),
              " elements: the basis functions of the coarser space are reproduced in the "
              "finer space only to ",
              error,
              ", which exceeds a tolerance of ",
              tolerance,
              ". A p-multigrid level transfer by element embedding requires the coarse space "
              "to be a subspace of the fine space.");
      }

  _embedding.copyToDeviceNested();
}

void
ElementEmbeddingTransfer::buildMultiplicity()
{
  _inverse_multiplicity.zero();

  _vectors[EMBEDDING_FINE].create(_inverse_multiplicity, _fine.dof_space, true);
  _vectors[EMBEDDING_FINE].copyToDevice();
  _vectors.copyToDevice();

  loops().multiplicity();

  _vectors[EMBEDDING_FINE].close();

  _inverse_multiplicity.close();

  // A DOF that no element of the fine space carries keeps a zero here, which leaves it untouched by
  // either direction of the transfer
  _inverse_multiplicity.reciprocal();
}

ElementEmbedding
ElementEmbeddingTransfer::loops() const
{
  return ElementEmbedding(_coarse.dof_space, _fine.dof_space, _embedding, _vectors);
}

void
ElementEmbeddingTransfer::prolong(const libMesh::NumericVector<Number> & x,
                                  libMesh::NumericVector<Number> & y)
{
  // The loops gather by local, ghost-inclusive DOF index, and assigning to a ghosted vector brings
  // its ghost values current
  _coarse_input = x;

  y.zero();

  // The output is scattered into, so it is wrapped for assembly: its ghost-row contributions are
  // buffered on device and added into the owning processes by close()
  _vectors[EMBEDDING_FINE].create(y, _fine.dof_space, true);
  _vectors[EMBEDDING_FINE].copyToDevice();
  _vectors[EMBEDDING_COARSE].create(_coarse_input, _coarse.dof_space, false);
  _vectors[EMBEDDING_COARSE].copyToDevice();
  _vectors.copyToDevice();

  loops().prolong();

  _vectors[EMBEDDING_FINE].close();
  _vectors[EMBEDDING_COARSE].restore();

  y.close();

  // Every element scatters the fine values of its own coarse DOFs, so a fine DOF shared by several
  // elements has received the same value once per element it belongs to
  y.pointwise_mult(y, _inverse_multiplicity);
}

void ElementEmbeddingTransfer::restrict(const libMesh::NumericVector<Number> & x,
                                        libMesh::NumericVector<Number> & y)
{
  // Prolongation weights an element-wise scatter after the fact, so the transpose weights the fine
  // vector ahead of the element-wise gather
  _fine_input.pointwise_mult(x, _inverse_multiplicity);

  y.zero();

  _vectors[EMBEDDING_COARSE].create(y, _coarse.dof_space, true);
  _vectors[EMBEDDING_COARSE].copyToDevice();
  _vectors[EMBEDDING_FINE].create(_fine_input, _fine.dof_space, false);
  _vectors[EMBEDDING_FINE].copyToDevice();
  _vectors.copyToDevice();

  loops().restrict();

  _vectors[EMBEDDING_COARSE].close();
  _vectors[EMBEDDING_FINE].restore();

  y.close();
}

} // namespace Moose::Kokkos
