//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVKernel.h"
#include "KokkosLinearFVBoundaryCondition.h"

#include "Attributes.h"
#include "FEProblemBase.h"
#include "PetscSupport.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVKernel::validParams()
{
  auto params = LinearSystemContributionObject::validParams();
  params += BlockRestrictable::validParams();
  params += NonADFunctorInterface::validParams();
  params.registerBase("LinearFVKernel");
  return params;
}

LinearFVKernel::LinearFVKernel(const InputParameters & parameters)
  : LinearSystemContributionObject(parameters), BlockRestrictable(this), NonADFunctorInterface(this)
{
  const auto & var = libMesh::cast_ref<MooseLinearVariableFV<Real> &>(
      _subproblem.getVariable(0,
                              getParam<LinearVariableName>("variable"),
                              Moose::VarKindType::VAR_SOLVER,
                              Moose::VarFieldType::VAR_FIELD_STANDARD));
  _kokkos_var.init(var, Moose::SOLUTION_TAG);

  // LinearFV does not need SNES options, not adding them will avoid petsc
  // unused option warnings.
  Moose::PetscSupport::dontAddCommonSNESOptions(_fe_problem);
}

LinearFVKernel::LinearFVKernel(const LinearFVKernel & object)
  : LinearSystemContributionObject(object),
    BlockRestrictable(object, {}),
    NonADFunctorInterface(object, {}),
    _kokkos_var(object._kokkos_var)
{
}

InputParameters
LinearFVElementalKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVElementalKernel");
  return params;
}

LinearFVElementalKernel::LinearFVElementalKernel(const InputParameters & parameters)
  : LinearFVKernel(parameters)
{
}

LinearFVElementalKernel::LinearFVElementalKernel(const LinearFVElementalKernel & object)
  : LinearFVKernel(object)
{
}

void
LinearFVElementalKernel::computeRightHandSide()
{
  Policy policy(0, numKokkosBlockElements());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVElementalKernel::computeMatrix()
{
  Policy policy(0, numKokkosBlockElements());
  if (!_matrix_dispatcher)
    _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
  _matrix_dispatcher->parallelFor(policy);
}

InputParameters
LinearFVFluxKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVFluxKernel");
  return params;
}

LinearFVFluxKernel::LinearFVFluxKernel(const InputParameters & parameters)
  : LinearFVKernel(parameters), FaceArgProducerInterface()
{
}

LinearFVFluxKernel::LinearFVFluxKernel(const LinearFVFluxKernel & object)
  : LinearFVKernel(object), FaceArgProducerInterface(), _bc_data(object._bc_data)
{
}

void
LinearFVFluxKernel::initialSetup()
{
  const auto n_elems = static_cast<dof_id_type>(kokkosMesh().getNumLocalElements());
  const auto max_sides = static_cast<dof_id_type>(_subproblem.mesh().getMaxSidesPerElem());
  _bc_data.matrix_coeff.create(max_sides, n_elems);
  _bc_data.rhs_coeff.create(max_sides, n_elems);
  _bc_data.matrix_coeff = 0;
  _bc_data.rhs_coeff = 0;

  std::vector<LinearFVBoundaryCondition *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_kokkos_var.sys())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(bcs);

  for (auto * bc : bcs)
    if (bc->variable().var() == _kokkos_var.var())
      bc->initBCData(_bc_data);
}

void
LinearFVFluxKernel::computeRightHandSide()
{
  Policy policy(0, numKokkosBlockSides());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVFluxKernel::computeMatrix()
{
  Policy policy(0, numKokkosBlockSides());
  if (!_matrix_dispatcher)
    _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
  _matrix_dispatcher->parallelFor(policy);
}

bool
LinearFVFluxKernel::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  const auto ft = fi.faceType(std::make_pair(_kokkos_var.var(), _kokkos_var.sys()));
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}

} // namespace Moose::Kokkos
