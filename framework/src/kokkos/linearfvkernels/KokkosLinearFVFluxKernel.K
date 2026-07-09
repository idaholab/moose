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
#include "MooseMesh.h"

#include <algorithm>

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
  : LinearFVKernel(parameters)
{
}

LinearFVFluxKernel::LinearFVFluxKernel(const LinearFVFluxKernel & object)
  : LinearFVKernel(object),
    _bc_data(object._bc_data),
    _bc_index(object._bc_index),
    _internal_face_ids(object._internal_face_ids),
    _boundary_face_ids(object._boundary_face_ids)
{
}

void
LinearFVFluxKernel::initialSetup()
{
  const auto n_elems =
      libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(kokkosMesh().getNumLocalElements());
  const auto max_sides =
      libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(_subproblem.mesh().getMaxSidesPerElem());
  _bc_index.create(max_sides, n_elems);
  _bc_index = -1;

  // Build a compact list of Kokkos linear FV BC data and a face lookup into that list.
  // Flux kernels use the lookup during face loops to fetch precomputed boundary
  // relations only on boundary faces where the kernel's variable has an active BC.
  const auto sys_num = _var.sys();
  const auto var_num = _var.var();
  const auto processor = processor_id();
  const auto var_name = _var.mooseVar()->name();

  std::vector<LinearFVBoundaryCondition *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(sys_num)
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(bcs);

  std::vector<LinearFVBoundaryCondition::BoundaryData> bc_data;
  // The face lookup is indexed by local Kokkos side/element IDs, so only populate
  // entries owned by this processor. Semi-local boundary element IDs can include
  // remote elements needed by other processors.
  auto & mesh = _subproblem.mesh();
  const auto * const kokkos_mesh = mesh.getKokkosMesh();

  for (auto * bc : bcs)
  {
    const auto & variable = bc->variable();

    if (variable.var() == var_num)
    {
      const auto bc_index = libMesh::cast_int<int>(bc_data.size());
      bc_data.push_back(bc->boundaryData());

      for (const auto boundary : bc->boundaryIDs())
        for (const auto elem_id : mesh.getBoundaryActiveSemiLocalElemIds(boundary))
        {
          const auto elem = mesh.elemPtr(elem_id);

          if (elem->processor_id() == processor)
          {
            const auto contiguous_elem = kokkos_mesh->getContiguousElementID(elem);
            const auto side = mesh.sideWithBoundaryID(elem, boundary);
            auto & existing_index = _bc_index(side, contiguous_elem);

            // Ambiguous boundary data would make flux kernels non-deterministic.
            if (existing_index != -1)
              mooseError("Kokkos linear FV flux kernel '",
                         name(),
                         "' found multiple Kokkos boundary conditions for variable '",
                         var_name,
                         "' on boundary ID ",
                         boundary,
                         ".");

            existing_index = bc_index;
          }
        }
    }
  }

  std::vector<FaceID> internal_face_ids;
  std::vector<FaceID> boundary_face_ids;

  const auto * const variable = _var.mooseVar();
  for (const auto block : blockIDs())
    for (const auto elem : mesh.getMesh().active_local_subdomain_elements_ptr_range(block))
    {
      const auto contiguous_elem = kokkos_mesh->getContiguousElementID(elem);

      for (const auto side : make_range(elem->n_sides()))
      {
        const auto * const neighbor = elem->neighbor_ptr(side);

        if (neighbor && neighbor != libMesh::remote_elem &&
            variable->activeOnSubdomain(neighbor->subdomain_id()))
          internal_face_ids.push_back({contiguous_elem, side});
        else if (_bc_index(side, contiguous_elem) != -1)
          boundary_face_ids.push_back({contiguous_elem, side});
      }
    }

  std::sort(internal_face_ids.begin(), internal_face_ids.end());
  std::sort(boundary_face_ids.begin(), boundary_face_ids.end());

  _internal_face_ids = internal_face_ids;
  _boundary_face_ids = boundary_face_ids;

  _bc_index.copyToDevice();
  _bc_data.create(libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(bc_data.size()));
  for (const auto i : index_range(bc_data))
    _bc_data[i] = bc_data[i];
  _bc_data.copyToDevice();

  // Non-Kokkos BCs are never called by Kokkos flux kernels; they would be silently
  // ignored, producing wrong results. Reject them early for this variable.
  std::vector<::LinearFVBoundaryCondition *> non_kokkos_bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(sys_num)
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(false)
      .template condition<AttribThread>(0)
      .queryInto(non_kokkos_bcs);

  for (auto * bc : non_kokkos_bcs)
  {
    const auto & variable = bc->variable();

    if (variable.number() == var_num)
      mooseError("Kokkos linear FV flux kernel '",
                 name(),
                 "' found non-Kokkos boundary condition '",
                 bc->name(),
                 "' (type '",
                 bc->type(),
                 "') for variable '",
                 variable.name(),
                 "'. Non-Kokkos BCs are not called by Kokkos flux kernels and will produce "
                 "incorrect results. Use the Kokkos equivalent, e.g. "
                 "'KokkosLinearFVFunctorDirichletBC'.");
  }
}

void
LinearFVFluxKernel::computeRightHandSide()
{
  if (hasInternalRightHandSideContribution() && _internal_face_ids.size())
  {
    Policy policy(0, _internal_face_ids.size());
    if (!_internal_rhs_dispatcher)
      _internal_rhs_dispatcher = DispatcherRegistry::build<InternalRightHandSideLoop>(this, type());
    _internal_rhs_dispatcher->parallelFor(policy);
  }

  if (_boundary_face_ids.size())
  {
    Policy policy(0, _boundary_face_ids.size());
    if (!_boundary_rhs_dispatcher)
      _boundary_rhs_dispatcher = DispatcherRegistry::build<BoundaryRightHandSideLoop>(this, type());
    _boundary_rhs_dispatcher->parallelFor(policy);
  }
}

void
LinearFVFluxKernel::computeMatrix()
{
  if (DispatcherRegistry::hasUserMethod<InternalMatrixLoop>(type()) && _internal_face_ids.size())
  {
    Policy policy(0, _internal_face_ids.size());
    if (!_internal_matrix_dispatcher)
      _internal_matrix_dispatcher = DispatcherRegistry::build<InternalMatrixLoop>(this, type());
    _internal_matrix_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<BoundaryMatrixLoop>(type()) && _boundary_face_ids.size())
  {
    Policy policy(0, _boundary_face_ids.size());
    if (!_boundary_matrix_dispatcher)
      _boundary_matrix_dispatcher = DispatcherRegistry::build<BoundaryMatrixLoop>(this, type());
    _boundary_matrix_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
