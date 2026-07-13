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
#include <map>

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

  // Build a compact list of Kokkos linear FV BC data and attach each BC worklist's packed index to
  // the boundary faces where this kernel's variable is active.
  const auto sys_num = _var.sys();
  const auto var_num = _var.var();
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
#ifdef DEBUG
  std::vector<LinearFVBoundaryCondition *> bc_objects;
#endif
  std::map<FaceID, BoundaryFaceID> bc_faces;
  auto & mesh = _subproblem.mesh();
  const auto * const kokkos_mesh = mesh.getKokkosMesh();

  for (auto * bc : bcs)
  {
    const auto & variable = bc->variable();

    if (variable.var() == var_num)
    {
      const auto bc_index = libMesh::cast_int<int>(bc_data.size());
      bc_data.push_back(bc->boundaryData());
#ifdef DEBUG
      bc_objects.push_back(bc);
#endif

      for (const auto bc_face_index : make_range(bc->numBoundaryFaces()))
      {
        const auto [contiguous_elem, side] = bc->boundaryFaceID(bc_face_index);
        const FaceID face{contiguous_elem, side};

#ifdef DEBUG
        if (bc_face_index)
          mooseAssert(
              bc->boundaryFaceID(bc_face_index - 1) < face,
              "Kokkos linear FV boundary face worklists must be ordered by (element, side)");
#endif

        auto & existing_index = _bc_index(side, contiguous_elem);

        // Ambiguous boundary data would make flux kernels non-deterministic.
        if (existing_index != -1)
          mooseError("Kokkos linear FV flux kernel '",
                     name(),
                     "' found multiple Kokkos boundary conditions for variable '",
                     var_name,
                     "' on contiguous element ",
                     contiguous_elem,
                     ", side ",
                     side,
                     ".");

        existing_index = bc_index;
        bc_faces.emplace(face, BoundaryFaceID{contiguous_elem, side, bc_index, bc_face_index});
      }
    }
  }

  std::vector<FaceID> internal_face_ids;
  std::vector<BoundaryFaceID> boundary_face_ids;

  const auto * const variable = _var.mooseVar();
  for (const auto block : blockIDs())
    for (const auto elem : mesh.getMesh().active_local_subdomain_elements_ptr_range(block))
    {
      const auto contiguous_elem = kokkos_mesh->getContiguousElementID(elem);

      for (const auto side : make_range(elem->n_sides()))
      {
        const auto * const neighbor = elem->neighbor_ptr(side);

        mooseAssert(!neighbor || neighbor != libMesh::remote_elem,
                    "We should have properly ghosted side neighbors for FV");
        if (neighbor && variable->activeOnSubdomain(neighbor->subdomain_id()))
          internal_face_ids.push_back({contiguous_elem, side});
        else
        {
          const auto bc_face = bc_faces.find({contiguous_elem, side});

          if (bc_face != bc_faces.end())
            boundary_face_ids.push_back(bc_face->second);
        }
      }
    }

  std::sort(internal_face_ids.begin(), internal_face_ids.end());
  std::sort(boundary_face_ids.begin(), boundary_face_ids.end());

#ifdef DEBUG
  for (const auto i : index_range(boundary_face_ids))
  {
    const auto & face = boundary_face_ids[i];
    mooseAssert(face.bc_index >= 0 &&
                    libMesh::cast_int<std::size_t>(face.bc_index) < bc_objects.size(),
                "Boundary face has an invalid compact boundary condition index");
    mooseAssert(_bc_index(face.side, face.elem) == face.bc_index,
                "Boundary face does not map to its expected compact boundary condition index");

    const auto * const bc = bc_objects[face.bc_index];
    mooseAssert(face.bc_face_index < bc->numBoundaryFaces(),
                "Boundary face has an invalid BC-local packed index");
    const auto expected_face = bc->boundaryFaceID(face.bc_face_index);
    mooseAssert(expected_face.first == face.elem && expected_face.second == face.side,
                "Boundary face does not map to its expected BC-local packed index");

    if (i && boundary_face_ids[i - 1].bc_index == face.bc_index)
      mooseAssert(boundary_face_ids[i - 1].bc_face_index < face.bc_face_index,
                  "Boundary faces and their BC-local packed indices have inconsistent ordering");
  }
#endif

  _internal_face_ids = internal_face_ids;
  _boundary_face_ids = boundary_face_ids;

#ifndef NDEBUG
  _bc_index.copyToDevice();
#endif
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
