//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegratedBCBase.h"

namespace Moose::Kokkos
{

InputParameters
IntegratedBCBase::validParams()
{
  auto params = BoundaryCondition::validParams();
  params += MaterialPropertyInterface::validParams();

  params.addParam<unsigned int>("num_local_residual_threads",
                                2,
                                "The number of threads for additional parallelization of local "
                                "DOFs in residual calculation.");
  params.addParam<unsigned int>("num_local_jacobian_threads",
                                4,
                                "The number of threads for additional parallelization of local "
                                "DOFs in Jacobian calculation.");

  // Integrated BCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

IntegratedBCBase::IntegratedBCBase(const InputParameters & parameters,
                                   Moose::VarFieldType field_type)
  : BoundaryCondition(parameters, field_type, false),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs())
#ifdef MOOSE_ENABLE_KOKKOS_GPU
    ,
    _num_local_residual_threads(getParam<unsigned int>("num_local_residual_threads")),
    _num_local_jacobian_threads(getParam<unsigned int>("num_local_jacobian_threads"))
#endif
{
#ifndef MOOSE_ENABLE_KOKKOS_GPU
  if (isParamSetByUser("num_local_residual_threads"))
    paramWarning("num_local_residual_threads", "Ignored when using Kokkos CPU backend.");
  if (isParamSetByUser("num_local_jacobian_threads"))
    paramWarning("num_local_jacobian_threads", "Ignored when using Kokkos CPU backend.");
#endif
}

IntegratedBCBase::IntegratedBCBase(const IntegratedBCBase & object)
  : BoundaryCondition(object),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    MaterialPropertyInterface(object, {}),
    _num_local_residual_threads(object._num_local_residual_threads),
    _num_local_jacobian_threads(object._num_local_jacobian_threads)
{
}

} // namespace Moose::Kokkos
