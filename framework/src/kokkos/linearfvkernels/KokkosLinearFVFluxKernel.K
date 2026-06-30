//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVFluxKernel.h"
#include "KokkosLinearFVBoundaryCondition.h"
#include "LinearFVBoundaryCondition.h"

#include "Attributes.h"
#include "FEProblemBase.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVFluxKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVFluxKernel");
  params.addPrivateParam<bool>("_kokkos_flux_kernel", true);
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
  const auto n_elems =
      libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(kokkosMesh().getNumLocalElements());
  const auto max_sides =
      libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(_subproblem.mesh().getMaxSidesPerElem());
  _bc_data.matrix_coeff.create(max_sides, n_elems);
  _bc_data.rhs_coeff.create(max_sides, n_elems);
  _bc_data.matrix_coeff = 0;
  _bc_data.rhs_coeff = 0;

  std::vector<LinearFVBoundaryCondition *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_var.sys())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(bcs);

  for (auto * bc : bcs)
    if (bc->variable().var() == _var.var())
      bc->initBCData(_bc_data);

  // Non-Kokkos BCs are never called by Kokkos flux kernels; they would be silently
  // ignored, producing wrong results (typically a zero RHS).
  std::vector<::LinearFVBoundaryCondition *> non_kokkos_bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_var.sys())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(false)
      .template condition<AttribThread>(0)
      .queryInto(non_kokkos_bcs);

  for (auto * bc : non_kokkos_bcs)
    if (bc->variable().number() == _var.var())
      mooseError("Kokkos linear FV flux kernel '",
                 name(),
                 "' found non-Kokkos boundary condition '",
                 bc->name(),
                 "' (type '",
                 bc->type(),
                 "') for variable '",
                 bc->variable().name(),
                 "'. Non-Kokkos BCs are not called by Kokkos flux kernels and will produce "
                 "incorrect results. Use the Kokkos equivalent, e.g. "
                 "'KokkosLinearFVAdvectionDiffusionFunctorDirichletBC'.");
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
  if (DispatcherRegistry::hasUserMethod<MatrixLoop>(type()))
  {
    Policy policy(0, numKokkosBlockSides());
    if (!_matrix_dispatcher)
      _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
    _matrix_dispatcher->parallelFor(policy);
  }
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
