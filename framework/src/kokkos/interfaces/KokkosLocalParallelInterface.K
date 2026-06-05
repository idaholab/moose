//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLocalParallelInterface.h"

namespace Moose::Kokkos
{

InputParameters
LocalParallelInterface::validParams()
{
  auto params = emptyInputParameters();

  // The default values are chosen empirically for first-order Lagrange shape functions in 3D
  // geometries. Higher-order shape functions can benefit from using more local threads, but the
  // default values will still provide moderate performance improvement. Residual calculation
  // shows negative impacts after certain thread count, so its default value is smaller than that of
  // Jacobian calculation.
  params.addParam<unsigned int>("num_local_residual_threads",
                                2,
                                "The number of threads for additional parallelization of local "
                                "DOFs in residual calculation.");
  params.addParam<unsigned int>("num_local_jacobian_threads",
                                4,
                                "The number of threads for additional parallelization of local "
                                "DOFs in Jacobian calculation.");

  return params;
}

LocalParallelInterface::LocalParallelInterface(const MooseObject * moose_object)
#ifdef MOOSE_ENABLE_KOKKOS_GPU
  : _num_local_residual_threads(moose_object->getParam<unsigned int>("num_local_residual_threads")),
    _num_local_jacobian_threads(moose_object->getParam<unsigned int>("num_local_jacobian_threads"))
#else
  : _num_local_residual_threads(1), _num_local_jacobian_threads(1)
#endif
{
#ifndef MOOSE_ENABLE_KOKKOS_GPU
  if (moose_object->isParamSetByUser("num_local_residual_threads"))
    moose_object->paramError("num_local_residual_threads",
                             "Should not be set when using Kokkos CPU backend.");
  if (moose_object->isParamSetByUser("num_local_jacobian_threads"))
    moose_object->paramError("num_local_jacobian_threads",
                             "Should not be set when using Kokkos CPU backend.");
#endif
}

} // namespace Moose::Kokkos
