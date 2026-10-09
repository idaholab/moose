//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLevelTransfer.h"
#include "KokkosVector.h"

#include "ConsoleStream.h"
#include "MooseError.h"

#include "libmesh/numeric_vector.h"

namespace Moose::Kokkos
{

void
LevelTransfer::fillVerificationPattern(libMesh::NumericVector<Number> & vector,
                                       const DofSpace & dof_space)
{
  Vector device;
  device.create(vector, dof_space, false);

  // Any pattern that varies from DOF to DOF exercises the transfer; deriving it from the global
  // index keeps a verification reproducible from run to run and independent of the partitioning
  device.setFromDofIndex(KOKKOS_LAMBDA(dof_id_type, const dof_id_type global)->PetscScalar {
    return 1.0 + ::Kokkos::sin(0.7 * (global + 1));
  });

  device.restore();
}

void
LevelTransfer::verify(const ConsoleStream & console)
{
  auto coarse = _coarse.sys.solution->zero_clone();
  auto fine = _fine.sys.solution->zero_clone();
  auto prolonged = _fine.sys.solution->zero_clone();
  auto restricted = _coarse.sys.solution->zero_clone();

  fillVerificationPattern(*coarse, _coarse.dof_space);
  fillVerificationPattern(*fine, _fine.dof_space);

  prolong(*coarse, *prolonged);
  restrictTo(*fine, *restricted);

  // The transpose relationship in inner-product form: (f, P c) = (P^T f, c) for every pair of
  // vectors, which is what a Galerkin coarse operator rests on
  const auto forward = fine->dot(*prolonged);
  const auto reverse = coarse->dot(*restricted);

  const auto error = std::abs(forward - reverse);

  // Both inner products sum the same products in a different order, so they agree to the roundoff
  // of those sums; the margin over the unit roundoff of double precision covers the accumulation
  const auto tolerance = 1e-10 * std::max(std::abs(forward), 1.0);

  console << "restriction is the transpose of prolongation to " << error << " (tolerance "
          << tolerance << ")" << std::endl;

  if (error > tolerance)
    mooseError("A p-multigrid level transfer's restriction is not the transpose of its "
               "prolongation: the inner products (f, P c) and (P^T f, c) differ by ",
               error,
               ", which exceeds a tolerance of ",
               tolerance,
               ".");
}

} // namespace Moose::Kokkos
