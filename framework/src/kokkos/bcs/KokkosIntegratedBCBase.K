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

  params.addParam<unsigned int>(
      "num_local_threads",
      4,
      "The number of threads for additional parallelization of local DOFs.");

  // Integrated BCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

IntegratedBCBase::IntegratedBCBase(const InputParameters & parameters,
                                   Moose::VarFieldType field_type)
  : BoundaryCondition(parameters, field_type, false),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _num_local_threads(getParam<unsigned int>("num_local_threads"))
{
}

IntegratedBCBase::IntegratedBCBase(const IntegratedBCBase & object)
  : BoundaryCondition(object),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    MaterialPropertyInterface(object, {}),
    _num_local_threads(object._num_local_threads)
{
}

} // namespace Moose::Kokkos
