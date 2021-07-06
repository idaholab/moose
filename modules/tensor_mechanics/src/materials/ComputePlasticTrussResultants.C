//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlasticTrussResultants.h"
#include "ComputeIncrementalTrussStrain.h"
#include "Function.h"
#include "MooseException.h"
#include "MathUtils.h"

registerMooseObject("TensorMechanicsApp", ComputePlasticTrussResultants);

InputParameters
ComputePlasticTrussResultants::validParams()
{
  InputParameters params = ComputeTrussResultants::validParams();
  params.addClassDescription(
      "Computes the stress and strain for a truss element with plastic behavior defined by either "
      "linear hardening or a user-defined hardening function.");
  params.addRequiredParam<Real>("yield_stress",
                                "Yield stress after which plastic strain starts accumulating");
  params.addParam<Real>("hardening_constant", 0.0, "Hardening slope");
  params.addParam<FunctionName>("hardening_function",
                                "Engineering stress as a function of plastic strain");
  params.addRequiredCoupledVar(
      "area",
      "Cross-section area of the truss. Can be supplied as either a number or a variable name.");
  params.addParam<Real>(
      "absolute_tolerance", 1e-10, "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>(
      "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  return params;
}

ComputePlasticTrussResultants::ComputePlasticTrussResultants(const InputParameters & parameters)
  : ComputeTrussResultants(parameters),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function")
                                                           : nullptr),
    _area(coupledValue("area")),
    _material_stiffness(getMaterialPropertyByName<Real>("material_stiffness")),
    _total_disp_strain(getMaterialProperty<RealVectorValue>("total_disp_strain")),
    _total_disp_strain_old(getMaterialPropertyOld<RealVectorValue>("total_disp_strain")),

    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),

    _plastic_strain(declareProperty<Real>("plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<Real>("plastic_strain")),

    _axial_stress(declareProperty<Real>("axial_stress")),
    _axial_stress_old(getMaterialPropertyOld<Real>("axial_stress")),
    _force(declareProperty<Real>("forces")),
    _force_old(getMaterialPropertyOld<Real>("forces")),
    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>("hardening_variable")),
    _max_its(1000)
{
  if (parameters.isParamSetByUser("hardening_constant") == isParamValid("hardening_function"))
    mooseError("ComputePlasticTrussResultants: Either hardening_constant or hardening_function "
               "must be defined");
}

void
ComputePlasticTrussResultants::initQpStatefulProperties()
{
  // ComputeIncrementalTrussStrain::initQpStatefulProperties();
  _plastic_strain[_qp] = 0.0;
  _hardening_variable[_qp] = 0.0;
}

void
ComputePlasticTrussResultants::computeQpProperties()
{
  Real strain_increment = _total_disp_strain[_qp](0) - _total_disp_strain_old[_qp](0);
  Real trial_stress = _axial_stress_old[_qp] + _material_stiffness[_qp] * strain_increment;

  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  Real yield_condition = std::abs(trial_stress) - _hardening_variable[_qp] - _yield_stress;
  Real iteration = 0;
  Real plastic_strain_increment = 0.0;
  Real elastic_strain_increment = strain_increment;

  if (yield_condition > 0.0)
  {
    Real residual = std::abs(trial_stress) - _hardening_variable[_qp] - _yield_stress -
                    _material_stiffness[_qp] * plastic_strain_increment;

    Real reference_residual =
        std::abs(trial_stress) - _material_stiffness[_qp] * plastic_strain_increment;

    while (std::abs(residual) > _absolute_tolerance ||
           std::abs(residual / reference_residual) > _relative_tolerance)
    {
      _hardening_variable[_qp] = computeHardeningValue(plastic_strain_increment);
      Real hardening_slope = computeHardeningDerivative(plastic_strain_increment);

      Real scalar = (std::abs(trial_stress) - _hardening_variable[_qp] - _yield_stress -
                     _material_stiffness[_qp] * plastic_strain_increment) /
                    (_material_stiffness[_qp] + hardening_slope);
      plastic_strain_increment += scalar;

      residual = std::abs(trial_stress) - _hardening_variable[_qp] - _yield_stress -
                 _material_stiffness[_qp] * plastic_strain_increment;

      reference_residual =
          std::abs(trial_stress) - _material_stiffness[_qp] * plastic_strain_increment;

      ++iteration;
      if (iteration > _max_its) // not converging
        throw MooseException("ComputePlasticTrussResultants: Plasticity model did not converge");
    }
    plastic_strain_increment *= MathUtils::sign(trial_stress);
    _plastic_strain[_qp] += plastic_strain_increment;
    elastic_strain_increment = strain_increment - plastic_strain_increment;
  }
  _axial_stress[_qp] = _axial_stress_old[_qp] + _material_stiffness[_qp] * elastic_strain_increment;
  _force[_qp] = _force_old[_qp] + _material_stiffness[_qp] * elastic_strain_increment * _area[_qp];
}

Real
ComputePlasticTrussResultants::computeHardeningValue(Real scalar)
{
  if (_hardening_function)
  {
    const Real strain_old = _plastic_strain_old[_qp];
    const Point p;

    return _hardening_function->value(std::abs(strain_old) + scalar, p) - _yield_stress;
  }

  return _hardening_variable_old[_qp] + _hardening_constant * scalar;
}

Real ComputePlasticTrussResultants::computeHardeningDerivative(Real /*scalar*/)
{
  if (_hardening_function)
  {
    const Real strain_old = _plastic_strain_old[_qp];
    const Point p;

    return _hardening_function->timeDerivative(std::abs(strain_old), p);
  }

  return _hardening_constant;
}
