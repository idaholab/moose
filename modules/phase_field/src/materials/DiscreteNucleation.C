//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleation.h"
#include "DiscreteNucleationMap.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleation);

InputParameters
DiscreteNucleation::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBase::validParams();
  params.addClassDescription("Free energy contribution for nucleating discrete particles");
  params.addRequiredCoupledVar("op_names",
                               "List of variables to force to a target concentration value");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  params.addRequiredParam<std::vector<Real>>("op_values", "List of target concentration values");
  params.addParam<Real>("penalty", 20.0, "Penalty factor for enforcing the target concentrations");
  MooseEnum penalty_mode("MATCH MIN MAX", "MATCH");
  params.addParam<MooseEnum>(
      "penalty_mode",
      penalty_mode,
      "Match the target concentration or take it as a minimum or maximum concentration target");
  return params;
}

DiscreteNucleation::DiscreteNucleation(const InputParameters & params)
  : DerivativeFunctionMaterialBase(params),
    _nvar(coupledComponents("op_names")),
    _op_index(_nvar),
    _op_values(getParam<std::vector<Real>>("op_values")),
    _penalty(getParam<Real>("penalty")),
    _penalty_mode(getParam<MooseEnum>("penalty_mode")),
    _map(getUserObject<DiscreteNucleationMap>("map"))
{
  // check inputs
  if (_nvar != _op_values.size())
    mooseError("The op_names and op_values parameter vectors must have the same number of entries");
  if (_nvar != _args.size())
    mooseError("Internal error.");

  // get libMesh variable numbers
  for (unsigned int i = 0; i < _nvar; ++i)
    _op_index[i] = argIndex(coupled("op_names", i));
}

void
DiscreteNucleation::computeProperties()
{
  // check if a nucleation event list is available for the current element
  const std::vector<Real> & nucleus = _map.nuclei(_current_elem);

  // calculate penalty
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // clear penalty value
    if (_prop_F)
      (*_prop_F)[_qp] = 0.0;

    for (unsigned int i = 0; i < _nvar; ++i)
    {
      const unsigned ii = _op_index[i];

      // modify the penalty magnitude with the nucleus mask
      const Real penalty = _penalty * nucleus[_qp];

      // deviation from the target concentration
      Real dc = (*_args[ii])[_qp] - _op_values[i];

      // ignore above/below target values for min/max modes respectively
      if ((_penalty_mode == 1 && dc > 0.0) || (_penalty_mode == 2 && dc < 0.0))
        dc = 0.0;

      // build free energy correction
      if (_prop_F)
        (*_prop_F)[_qp] += dc * dc * penalty;

      // first derivative
      if (_prop_dF[ii])
        (*_prop_dF[ii])[_qp] = 2.0 * dc * penalty;

      // second derivatives
      for (unsigned int jj = ii; jj < _nvar; ++jj)
      {
        if (_prop_d2F[ii][jj])
          (*_prop_d2F[ii][jj])[_qp] = 2.0 * penalty;

        // third derivatives
        if (_third_derivatives)
          for (unsigned int kk = jj; kk < _nvar; ++kk)
            if (_prop_d3F[ii][jj][kk])
              (*_prop_d3F[ii][jj][kk])[_qp] = 0.0;
      }
    }
  }
}
