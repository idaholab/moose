//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledValueFunctionFreeEnergy.h"
#include "Function.h"

registerMooseObject("PhaseFieldApp", CoupledValueFunctionFreeEnergy);

InputParameters
CoupledValueFunctionFreeEnergy::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBase::validParams();
  params.addClassDescription("Compute a free energy from a lookup function");
  params.addRequiredParam<FunctionName>("free_energy_function",
                                        "Coupled function to evaluate with values from v");
  params.addRequiredParam<std::vector<FunctionName>>(
      "chemical_potential_functions", "Coupled function to evaluate with values from v");
  params.addCoupledVar("v",
                       "List of up to four coupled variables that are substituted for x,y,z, and t "
                       "in the coupled function");
  params.set<unsigned int>("derivative_order") = 2;
  return params;
}

CoupledValueFunctionFreeEnergy::CoupledValueFunctionFreeEnergy(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters),
    _free_energy_function(
        isParamValid("free_energy_function") ? &getFunction("free_energy_function") : nullptr),
    _chemical_potential_names(getParam<std::vector<FunctionName>>("chemical_potential_functions")),
    _chemical_potential_functions(_nargs)
{
  if (_chemical_potential_functions.size() != _nargs)
    paramError(
        "chemical_potential_functions",
        "Exactly one chemical potential function must be supplied for each coupled variable");

  // get chemical potential functions
  for (unsigned int i = 0; i < _nargs; ++i)
    _chemical_potential_functions[i] = &getFunctionByName(_chemical_potential_names[i]);
}

void
CoupledValueFunctionFreeEnergy::initialSetup()
{
  if (_prop_F && !_free_energy_function)
    paramError("free_energy_function",
               "The undifferentiated free energy property is requested in the simulation, but no "
               "function is provided");
}

void
CoupledValueFunctionFreeEnergy::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // set function arguments to coupled variables
    Point p;
    Real t = 0;
    for (unsigned int i = 0; i < 3 && i < _nargs; ++i)
      p(i) = (*_args[i])[_qp];
    if (_nargs == 4)
      t = (*_args[3])[_qp];

    // set function value
    if (_prop_F && _free_energy_function)
      (*_prop_F)[_qp] = _free_energy_function->value(t, p);

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      // set first derivatives
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = _chemical_potential_functions[i]->value(t, p);

      // second derivatives via grad
      auto grad = _chemical_potential_functions[i]->gradient(t, p);
      for (unsigned int j = i; j < 3 && j < _nargs; ++j)
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = grad(j);

      // second derivative via time derivative
      if (_nargs == 4 && _prop_d2F[i][3])
        (*_prop_d2F[i][3])[_qp] = _chemical_potential_functions[i]->timeDerivative(t, p);
    }
  }
}
