//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVPressureJumpGreenGaussGradient.h"

#include "FEProblemBase.h"
#include "FVUtils.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearSystem.h"
#include "MooseLinearVariableFV.h"
#include "MooseMesh.h"
#include "PetscVectorReader.h"
#include "PressureJumpInterface.h"
#include "RhieChowMassFlux.h"

#include "libmesh/elem.h"

void
FVPressureJumpGreenGaussGradient::clear()
{
  _gradient.clear();
  _pressure_generation = libMesh::DofObject::invalid_id;
  _jump_generation = libMesh::DofObject::invalid_id;
}

bool
FVPressureJumpGreenGaussGradient::current(const RhieChowMassFlux & rc) const
{
  return !_gradient.empty() && _pressure_generation == rc.pressureSolutionGeneration() &&
         _jump_generation == rc.baffleJumpGeneration();
}

RealVectorValue
FVPressureJumpGreenGaussGradient::gradient(const RhieChowMassFlux & rc,
                                           const ElemInfo & elem_info) const
{
  mooseAssert(current(rc),
              "A pressure-jump interface requested gradients from incompatible pressure or jump "
              "generations.");

  const auto dof =
      elem_info.dofIndices()[rc.globalPressureSystemNumber()][rc.pressureVariableNumber()];
  RealVectorValue value;
  for (const auto component : index_range(_gradient))
    value(component) = (*_gradient[component])(dof);
  return value;
}

void
FVPressureJumpGreenGaussGradient::reconstruct(RhieChowMassFlux & rc)
{
  auto & pressure_system = rc.pressureSystem();
  auto & fe_problem = pressure_system.feProblem();
  auto & mesh = fe_problem.mesh();
  const auto dimension = rc.dimension();
  const auto pressure_variable_number = rc.pressureVariableNumber();
  const auto system_number = rc.globalPressureSystemNumber();
  const auto & pressure_variable = rc.pressureVariable();
  auto & solution = *pressure_system.system().current_local_solution;

  GradientContainer lagged_gradient;
  lagged_gradient.reserve(_gradient.size());
  for (const auto & component : _gradient)
    lagged_gradient.push_back(component->clone());

  if (_gradient.empty())
  {
    _gradient.reserve(dimension);
    for (const auto component : make_range(dimension))
    {
      libmesh_ignore(component);
      _gradient.push_back(solution.zero_clone());
    }
  }
  else
    for (auto & component : _gradient)
      component->zero();

  PetscVectorReader pressure_reader(solution);

  for (auto face_iterator = mesh.ownedFaceInfoBegin(); face_iterator != mesh.ownedFaceInfoEnd();
       ++face_iterator)
  {
    const auto * const fi = *face_iterator;
    const auto face_type = fi->faceType(std::make_pair(pressure_variable_number, system_number));

    if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[system_number][pressure_variable_number];
      const auto neighbor_dof = neighbor_info.dofIndices()[system_number][pressure_variable_number];
      const Real elem_pressure = pressure_reader(elem_dof);
      const Real neighbor_pressure = pressure_reader(neighbor_dof);

      Real elem_trace;
      Real neighbor_trace;
      if (rc.faceIsBaffle(*fi))
      {
        RealVectorValue elem_gradient;
        RealVectorValue neighbor_gradient;
        if (!lagged_gradient.empty())
          for (const auto component : make_range(dimension))
          {
            elem_gradient(component) = (*lagged_gradient[component])(elem_dof);
            neighbor_gradient(component) = (*lagged_gradient[component])(neighbor_dof);
          }

        RealVectorValue elem_diffusion;
        RealVectorValue neighbor_diffusion;
        for (const auto component : make_range(dimension))
        {
          elem_diffusion(component) = rc.cellPressureDiffusionCoefficient(elem_info, component);
          neighbor_diffusion(component) =
              rc.cellPressureDiffusionCoefficient(neighbor_info, component);
        }

        const Point elem_to_face = fi->faceCentroid() - elem_info.centroid();
        const Point face_to_neighbor = neighbor_info.centroid() - fi->faceCentroid();
        const Real face_area = fi->faceArea() * fi->faceCoord();
        const auto interface_data =
            NS::FV::pressureJumpInterfaceData(fi->normal(),
                                              elem_to_face,
                                              face_to_neighbor,
                                              elem_diffusion,
                                              neighbor_diffusion,
                                              elem_gradient,
                                              neighbor_gradient,
                                              face_area,
                                              rc.pressureDiffusionUsesNonorthogonalCorrection());

        if (!interface_data.valid)
          mooseError("Cannot reconstruct jump-aware pressure gradients on face ID ",
                     fi->id(),
                     ": both half-cell normal conductances must be finite and positive.");

        const Real jump = rc.getSignedBaffleJump(*fi, /*elem_side=*/true);
        const Real flux =
            NS::FV::pressureJumpFlux(interface_data, elem_pressure, neighbor_pressure, jump);
        const auto traces =
            NS::FV::pressureJumpFaceTraces(interface_data, elem_pressure, neighbor_pressure, flux);
        elem_trace = traces.elem;
        neighbor_trace = traces.neighbor;
      }
      else
      {
        const auto weights = Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *fi, true);
        elem_trace = neighbor_trace =
            weights.first * elem_pressure + weights.second * neighbor_pressure;
      }

      const auto surface_vector = fi->normal() * fi->faceArea() * fi->faceCoord();
      for (const auto component : make_range(dimension))
      {
        _gradient[component]->add(elem_dof, surface_vector(component) * elem_trace);
        _gradient[component]->add(neighbor_dof, -surface_vector(component) * neighbor_trace);
      }
    }
    else if (face_type == FaceInfo::VarFaceNeighbors::ELEM ||
             face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    {
      auto * const bc = pressure_variable.getBoundaryCondition(*fi->boundaryIDs().begin());
      if (bc)
        bc->setupFaceData(fi, face_type);

      const bool elem_side = face_type == FaceInfo::VarFaceNeighbors::ELEM;
      const auto & elem_info = elem_side ? *fi->elemInfo() : *fi->neighborInfo();
      const auto dof = elem_info.dofIndices()[system_number][pressure_variable_number];
      const Real face_value = bc ? bc->computeBoundaryValue() : pressure_reader(dof);
      const Real orientation = elem_side ? 1.0 : -1.0;
      const auto surface_vector = orientation * fi->normal() * fi->faceArea() * fi->faceCoord();
      for (const auto component : make_range(dimension))
        _gradient[component]->add(dof, surface_vector(component) * face_value);
    }
  }

  for (auto & component : _gradient)
    component->close();

  const auto radial_coordinate = mesh.getAxisymmetricRadialCoord();
  for (auto elem_iterator = mesh.ownedElemInfoBegin(); elem_iterator != mesh.ownedElemInfoEnd();
       ++elem_iterator)
  {
    const auto * const elem_info = *elem_iterator;
    if (!rc.hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto dof = elem_info->dofIndices()[system_number][pressure_variable_number];
    const Real volume = elem_info->volume() * elem_info->coordFactor();
    for (const auto component : make_range(dimension))
      _gradient[component]->set(dof, (*_gradient[component])(dof) / volume);

    if (mesh.getCoordSystem(elem_info->subdomain_id()) == Moose::CoordinateSystemType::COORD_RZ)
    {
      mooseAssert(elem_info->centroid()(radial_coordinate) != 0.0,
                  "Axisymmetric control volumes must not have a zero radial coordinate.");
      _gradient[radial_coordinate]->add(
          dof, -pressure_reader(dof) / elem_info->centroid()(radial_coordinate));
    }
  }

  for (auto & component : _gradient)
    component->close();

  _pressure_generation = rc.pressureSolutionGeneration();
  _jump_generation = rc.baffleJumpGeneration();
}
