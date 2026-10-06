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
#include "PressureJumpInterface.h"
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
  _pressure_gradient = &pressure_gradient;
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

  // Assemble the Green-Gauss pressure surface integral. Internal baffle faces need different
  // pressure values on their two sides so the prescribed jump is not smeared into either cell.
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
        mooseAssert(_pressure_gradient,
                    "A linked pressure gradient is required on pressure-jump faces.");

        const Real jump = _rhie_chow->getSignedBaffleJump(*fi, /*elem_side=*/true);
        if (!_rhie_chow->pressureDiffusionDataReady())
        {
          // Gradient history is initialized before the pressure-diffusion coefficients exist.
          // Preserve the jump during this startup pass using jump-adjusted interpolation.
          elem_face_pressure =
              Moose::FV::linearInterpolation(elem_pressure, neighbor_pressure + jump, *fi, true);
          neighbor_face_pressure =
              Moose::FV::linearInterpolation(elem_pressure - jump, neighbor_pressure, *fi, true);
        }
        else
        {
          // The cell coefficients are the diagonal entries of the pressure-diffusion tensor.
          RealVectorValue elem_diffusion;
          RealVectorValue neighbor_diffusion;
          for (const auto component : make_range(dimension))
          {
            elem_diffusion(component) =
                _rhie_chow->cellPressureDiffusionCoefficient(elem_info, component);
            neighbor_diffusion(component) =
                _rhie_chow->cellPressureDiffusionCoefficient(neighbor_info, component);
          }

          // Use the published geometric pressure gradient to match the deferred correction in the
          // pressure equation. The reconstructed coupling gradient must not enter this correction.
          const auto interface_data = NS::FV::pressureJumpInterfaceData(
              fi->normal(),
              fi->faceCentroid() - elem_info.centroid(),
              neighbor_info.centroid() - fi->faceCentroid(),
              elem_diffusion,
              neighbor_diffusion,
              _pressure_gradient->gradient(elem_info),
              _pressure_gradient->gradient(neighbor_info),
              fi->faceArea() * fi->faceCoord(),
              _rhie_chow->pressureDiffusionUsesNonorthogonalCorrection());

          if (interface_data.valid)
          {
            // Eliminate the interface pressures to obtain one conservative flux, then recover the
            // one-sided face value used in each cell's Green-Gauss surface sum.
            const Real flux =
                NS::FV::pressureJumpFlux(interface_data, elem_pressure, neighbor_pressure, jump);
            elem_face_pressure = NS::FV::pressureJumpOneSidedFaceValue(
                interface_data, elem_pressure, flux, /*elem_side=*/true);
            neighbor_face_pressure = NS::FV::pressureJumpOneSidedFaceValue(
                interface_data, neighbor_pressure, flux, /*elem_side=*/false);
          }
          else
          {
            // Degenerate half-cell geometry cannot define a conductance. The interpolation
            // fallback still preserves the prescribed pressure jump.
            elem_face_pressure =
                Moose::FV::linearInterpolation(elem_pressure, neighbor_pressure + jump, *fi, true);
            neighbor_face_pressure =
                Moose::FV::linearInterpolation(elem_pressure - jump, neighbor_pressure, *fi, true);
          }
        }
      }
      else
      {
        // A continuous internal face has one shared pressure value.
        elem_face_pressure = neighbor_face_pressure =
            Moose::FV::linearInterpolation(elem_pressure, neighbor_pressure, *fi, true);
      }

      // FaceInfo normals point out of the element, hence the opposite sign for the neighbor row.
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
      // A boundary face contributes only to the cell on the side where the variable is defined.
      auto * const bc = pressure_variable->getBoundaryCondition(*fi->boundaryIDs().begin());
      if (bc)
        bc->setupFaceData(fi, face_type);

      const bool elem_side = face_type == FaceInfo::VarFaceNeighbors::ELEM;
      const auto & elem_info = elem_side ? *fi->elemInfo() : *fi->neighborInfo();
      const auto dof = elem_info.dofIndices()[system_number][pressure_variable_number];
      // Without an explicit boundary condition, extrapolate the adjacent cell pressure.
      const Real face_value = bc ? bc->computeBoundaryValue() : pressure_reader(dof);
      const Real orientation = elem_side ? 1.0 : -1.0;
      const auto surface_vector = orientation * fi->normal() * fi->faceArea() * fi->faceCoord();
      for (const auto component : make_range(dimension))
        gradient[component]->add(dof, surface_vector(component) * face_value);
    }
  }

  // Complete the distributed surface sums before applying the cell-volume normalization.
  for (auto & component : gradient)
    component->close();

  PARALLEL_TRY
  {
    // Convert each accumulated integral to a gradient using the coordinate-system-aware volume.
    using ElemInfoRange = ComputeLinearFVGreenGaussGradientVolumeThread::ElemInfoRange;
    ElemInfoRange elem_info_range(mesh.ownedElemInfoBegin(), mesh.ownedElemInfoEnd());
    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem, system, gradient, variable_numbers);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();
}
