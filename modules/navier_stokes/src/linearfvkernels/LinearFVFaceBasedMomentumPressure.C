//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVFaceBasedMomentumPressure.h"
#include "FEProblemBase.h"
#include "NS.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", LinearFVFaceBasedMomentumPressure);

InputParameters
LinearFVFaceBasedMomentumPressure::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the reduced-pressure momentum predictor forcing reconstructed from face-based "
      "pressure/hydrostatic/capillary data.");
  params.addParam<VariableName>(NS::pressure,
                                "The pressure variable whose gradient should be used for the "
                                "boundary closure of the face-based predictor forcing.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The sharp-interface Rhie-Chow user object that reconstructs the face-based predictor "
      "forcing.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

LinearFVFaceBasedMomentumPressure::LinearFVFaceBasedMomentumPressure(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _pressure_var(getPressureVariable(NS::pressure)),
    _sharp_mass_flux_provider(
        getUserObject<ConservativeSharpInterfaceRhieChowMassFluxBase>("rhie_chow_user_object"))
{
  _pressure_var.computeCellGradients();
}

MooseLinearVariableFV<Real> &
LinearFVFaceBasedMomentumPressure::getPressureVariable(const std::string & vname)
{
  auto * ptr = dynamic_cast<MooseLinearVariableFV<Real> *>(
      &_fe_problem.getVariable(_tid, getParam<VariableName>(vname)));

  if (!ptr)
    paramError(NS::pressure, "The pressure variable should be of type MooseLinearVariableFVReal!");

  return *ptr;
}

Real
LinearFVFaceBasedMomentumPressure::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVFaceBasedMomentumPressure::computeRightHandSideContribution()
{
  return _sharp_mass_flux_provider
             .reducedPressureMomentumPredictorForceDensity(*_current_elem_info, Moose::currentState())(
             _index) *
         _current_elem_volume;
}
