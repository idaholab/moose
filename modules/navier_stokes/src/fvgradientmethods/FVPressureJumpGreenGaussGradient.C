//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVPressureJumpGreenGaussGradient.h"

#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "FEProblemBase.h"
#include "FVUtils.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVGradientReader.h"
#include "LinearSystem.h"
#include "MooseLinearVariableFV.h"
#include "MooseMesh.h"
#include "PetscVectorReader.h"
#include "PorousRhieChowMassFlux.h"
#include "SystemBase.h"

#include "libmesh/elem.h"

registerMooseObject("NavierStokesApp", FVPressureJumpGreenGaussGradient);

InputParameters
FVPressureJumpGreenGaussGradient::validParams()
{
  InputParameters params = FVGradientMethod::validParams();
  params.suppressParameter<MooseEnum>("limiter");
  params.addClassDescription(
      "Computes a Green-Gauss pressure gradient without smearing prescribed baffle jumps.");
  return params;
}

FVPressureJumpGreenGaussGradient::FVPressureJumpGreenGaussGradient(const InputParameters & params)
  : FVGradientMethod(params)
{
}

void
FVPressureJumpGreenGaussGradient::linkFlowSystem(PorousRhieChowMassFlux & rc,
                                                 const LinearFVGradientReader & pressure_gradient)
{
  if (&pressure_gradient.method() != this)
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' must be linked using a gradient field produced by that method.");

  if (&pressure_gradient.system() != &rc.pressureSystem() ||
      pressure_gradient.variableNumber() != rc.pressureVariableNumber())
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' must be linked to the pressure variable owned by PorousRhieChowMassFlux '",
               rc.name(),
               "'.");

  if (_rhie_chow && _rhie_chow != &rc)
    mooseError("FVPressureJumpGreenGaussGradient '",
               name(),
               "' is already linked to PorousRhieChowMassFlux '",
               _rhie_chow->name(),
               "'. Use a separate gradient method for each flow system.");

  _rhie_chow = &rc;
  _pressure_system = &pressure_gradient.system();
  _pressure_variable_number = pressure_gradient.variableNumber();
}

void
FVPressureJumpGreenGaussGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  mooseAssert(variable_numbers.size() == 1,
              "FVPressureJumpGreenGaussGradient must be registered for exactly one pressure "
              "variable.");
  const auto pressure_variable_number = *variable_numbers.begin();

  if (_pressure_system)
  {
    mooseAssert(_pressure_system == &system,
                "FVPressureJumpGreenGaussGradient can only compute gradients for its linked "
                "pressure system.");
    mooseAssert(_pressure_variable_number == pressure_variable_number,
                "FVPressureJumpGreenGaussGradient can only compute the linked pressure variable.");
  }

  auto * const pressure_variable =
      dynamic_cast<MooseLinearVariableFVReal *>(&system.getVariable(0, pressure_variable_number));
  mooseAssert(pressure_variable,
              "FVPressureJumpGreenGaussGradient requires a MooseLinearVariableFVReal.");

  auto & fe_problem = system.feProblem();
  auto & mesh = fe_problem.mesh();
  const auto dimension = mesh.dimension();
  const auto system_number = system.number();
  PetscVectorReader pressure_reader(*system.system().current_local_solution);

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

      Real elem_face_pressure;
      Real neighbor_face_pressure;
      if (_rhie_chow && _rhie_chow->faceIsBaffle(*fi))
      {
        const Real jump = _rhie_chow->getSignedBaffleJump(*fi, /*elem_side=*/true);
        elem_face_pressure =
            Moose::FV::linearInterpolation(elem_pressure, neighbor_pressure + jump, *fi, true);
        neighbor_face_pressure =
            Moose::FV::linearInterpolation(elem_pressure - jump, neighbor_pressure, *fi, true);
      }
      else
      {
        elem_face_pressure = neighbor_face_pressure =
            Moose::FV::linearInterpolation(elem_pressure, neighbor_pressure, *fi, true);
      }

      const auto surface_vector = fi->normal() * fi->faceArea() * fi->faceCoord();
      for (const auto component : make_range(dimension))
      {
        gradient[component]->add(elem_dof, surface_vector(component) * elem_face_pressure);
        gradient[component]->add(neighbor_dof, -surface_vector(component) * neighbor_face_pressure);
      }
    }
    else if (face_type == FaceInfo::VarFaceNeighbors::ELEM ||
             face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    {
      auto * const bc = pressure_variable->getBoundaryCondition(*fi->boundaryIDs().begin());
      if (bc)
        bc->setupFaceData(fi, face_type);

      const bool elem_side = face_type == FaceInfo::VarFaceNeighbors::ELEM;
      const auto & elem_info = elem_side ? *fi->elemInfo() : *fi->neighborInfo();
      const auto dof = elem_info.dofIndices()[system_number][pressure_variable_number];
      const Real face_value = bc ? bc->computeBoundaryValue() : pressure_reader(dof);
      const Real orientation = elem_side ? 1.0 : -1.0;
      const auto surface_vector = orientation * fi->normal() * fi->faceArea() * fi->faceCoord();
      for (const auto component : make_range(dimension))
        gradient[component]->add(dof, surface_vector(component) * face_value);
    }
  }

  for (auto & component : gradient)
    component->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = ComputeLinearFVGreenGaussGradientVolumeThread::ElemInfoRange;
    ElemInfoRange elem_info_range(mesh.ownedElemInfoBegin(), mesh.ownedElemInfoEnd());
    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem, system, gradient, variable_numbers);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();
}
