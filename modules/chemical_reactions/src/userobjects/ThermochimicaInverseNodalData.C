//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaInverseNodalData.h"

#include "ThermochimicaUtils.h"
#include "NestedSolve.h"

registerMooseObject("ChemicalReactionsApp", ThermochimicaInverseNodalData);

InputParameters
ThermochimicaInverseNodalData::validParams()
{
  InputParameters params = ThermochimicaNodalBase::validParams();
  ThermochimicaUtils::addClassDescription(
      params, "Provides access to Thermochimica-calculated data at nodes.");

  params += NestedSolve::validParams();

  params.addRequiredCoupledVar("chemical_potential", "Coupled chemical_potential");
  params.addRequiredParam<int>("which_mu",
                               "Index of element for Which chemical potential to match");
  params.addParam<Real>(
      "finite_difference_width", 0.01, "Width of finite difference to use for Newton Iteration");
  params.addParam<bool>("verbose", false, "Flag to output verbose information");

  return params;
}

ThermochimicaInverseNodalData::ThermochimicaInverseNodalData(const InputParameters & parameters)
  : ThermochimicaNodalBase(parameters),
    _el_writable(_n_elements),
    _mu(coupledValue("chemical_potential")),
    _nested_solve(NestedSolve(parameters)),
    _which_mu(getParam<int>("which_mu")),
    _finite_difference_width(getParam<Real>("finite_difference_width")),
    _verbose(getParam<bool>("verbose"))
{
  for (const auto i : make_range(_n_elements))
  {
    _el_writable[i] = &writableVariable("elements", i);
    if (i == _which_mu)
      _el[i] = &coupledValueOld("elements", i);
    else
      _el[i] = &coupledValue("elements", i);
  }
  if (!_output_element_potentials)
    paramError("output_element_potentials", "Element potentials outputs must be provided");
}

void
ThermochimicaInverseNodalData::execute()
{
#ifdef THERMOCHIMICA_ENABLED

  // Initialize error bool
  int idbg = 0;

  // Set all elements to variable values.
  // At the same time, set initial guess for changing concentration to previous solution
  std::vector<Real> element_values(_n_elements);
  for (const auto i : make_range(_n_elements))
    element_values[i] = (*_el[i])[_qp];

  NestedSolve::Value<> solution(1);
  solution << element_values[_which_mu];

  if (_verbose)
  {
    Moose::out << "Starting Newton loop\n  Initial guess: " << solution(0)
               << "\n  Element values\n";
    for (const auto i : make_range(_n_elements))
      Moose::out << "    i: " << i << " val: " << element_values[i] << "\n";
  }

  auto computeResidual = [&](const NestedSolve::Value<> & guess, NestedSolve::Value<> & residual)
  {
    element_values[_which_mu] = guess(0);

    idbg = setParamsAndRun(element_values);
    if (idbg)
      mooseError("Thermochimica error ", idbg);
    residual(0) = getElementalChemicalPotentials(_which_mu) - _mu[_qp];

    if (_verbose)
      Moose::out << "  guess: " << guess(0) << " pot: " << getElementalChemicalPotentials(_which_mu)
                 << " mu: " << _mu[_qp] << " res: " << residual(0) << std::endl;
  };

  auto computeJacobian = [&](const NestedSolve::Value<> & guess, NestedSolve::Jacobian<> & jacobian)
  {
    element_values[_which_mu] = guess(0) * (1.0 + _finite_difference_width / 2.0);

    idbg = setParamsAndRun(element_values);
    if (idbg)
      mooseError("Thermochimica error ", idbg);
    jacobian(0) = getElementalChemicalPotentials(_which_mu);

    if (_verbose)
      Moose::out << "    high: " << jacobian(0) << " at " << element_values[_which_mu];

    element_values[_which_mu] = guess(0) * (1.0 - _finite_difference_width / 2.0);
    idbg = setParamsAndRun(element_values);
    if (idbg)
      mooseError("Thermochimica error ", idbg);
    jacobian(0) -= getElementalChemicalPotentials(_which_mu);

    jacobian(0) /= _finite_difference_width;

    // Reset element_values back to correct guess
    element_values[_which_mu] = guess(0);

    if (_verbose)
      Moose::out << "\n    low: " << getElementalChemicalPotentials(_which_mu) << " at "
                 << element_values[_which_mu] << "\n    jac: " << jacobian(0) << std::endl;
  };

  _nested_solve.nonlinear(solution, computeResidual, computeJacobian);

  if (_nested_solve.getState() == NestedSolve::State::NOT_CONVERGED)
    mooseException("In ", _name, ": inverse solve not converged");
  else if (_nested_solve.getState() == NestedSolve::State::CONVERGED_ABS ||
           _nested_solve.getState() == NestedSolve::State::CONVERGED_REL ||
           _nested_solve.getState() == NestedSolve::State::EXACT_GUESS)
  {
    if (_verbose)
      Moose::out << "converged at " << element_values[_which_mu] << std::endl;

    // Write elemental values
    for (const auto i : make_range(_n_elements))
      _el_writable[i]->setNodalValue(element_values[i], _qp);

    // Save data for future reinits
    reinitDataMooseFromTc();

    // Save outputs
    setOutputNodalValues();
  }

#endif
}
