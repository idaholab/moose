//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "LinearFVConservativeEnergyDiffusion.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("NavierStokesApp", LinearFVConservativeEnergyDiffusion);

InputParameters
LinearFVConservativeEnergyDiffusion::validParams()
{
  InputParameters params = LinearFVDiffusion::validParams();
  params.addClassDescription("Diffuses T = E / (rho * cp) while solving for conserved thermal "
                             "energy E = rho * cp * T.");
  params.addRequiredParam<MooseFunctorName>("rho_cp", "Cell-centered rho * cp functor.");
  return params;
}

LinearFVConservativeEnergyDiffusion::LinearFVConservativeEnergyDiffusion(
    const InputParameters & params)
  : LinearFVDiffusion(params), _rho_cp(getFunctor<Real>("rho_cp"))
{
  if (_use_nonorthogonal_correction)
    paramError("use_nonorthogonal_correction",
               "Non-orthogonal correction is not supported for conserved energy diffusion.");
}

Real
LinearFVConservativeEnergyDiffusion::elemInverseRhoCp() const
{
  return 1.0 / _rho_cp(makeElemArg(_current_face_info->elemPtr()), determineState());
}

Real
LinearFVConservativeEnergyDiffusion::neighborInverseRhoCp() const
{
  return 1.0 / _rho_cp(makeElemArg(_current_face_info->neighborPtr()), determineState());
}

Real
LinearFVConservativeEnergyDiffusion::singleSidedInverseRhoCp() const
{
  const auto elem = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                        ? _current_face_info->elemPtr()
                        : _current_face_info->neighborPtr();
  return 1.0 / _rho_cp(makeElemArg(elem), determineState());
}

Real
LinearFVConservativeEnergyDiffusion::computeElemMatrixContribution()
{
  return computeFluxMatrixContribution() * elemInverseRhoCp();
}

Real
LinearFVConservativeEnergyDiffusion::computeNeighborMatrixContribution()
{
  return -computeFluxMatrixContribution() * neighborInverseRhoCp();
}

Real
LinearFVConservativeEnergyDiffusion::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVConservativeEnergyDiffusion::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVConservativeEnergyDiffusion::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(diff_bc, "This should be a valid BC!");

  auto grad_contrib = diff_bc->computeBoundaryGradientMatrixContribution() * _current_face_area;
  if (!diff_bc->includesMaterialPropertyMultiplier())
    grad_contrib *= _diffusion_coeff(singleSidedFaceArg(_current_face_info), determineState()) *
                    singleSidedInverseRhoCp();

  return grad_contrib;
}
