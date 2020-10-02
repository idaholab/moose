//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledBEKinetic.h"

registerMooseObject("ChemicalReactionsApp", CoupledBEKinetic);

InputParameters
CoupledBEKinetic::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addRequiredParam<std::vector<Real>>("weight",
                                             "The weight of kinetic species concentration");
  params.addCoupledVar("v", "List of kinetic species being coupled by concentration");
  params.addClassDescription("Derivative of kinetic species concentration wrt time");
  return params;
}

CoupledBEKinetic::CoupledBEKinetic(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _porosity(getMaterialProperty<Real>("porosity")),
    _weight(getParam<std::vector<Real>>("weight")),
    _vals(coupledValues("v")),
    _vals_old(coupledValuesOld("v"))
{
}

Real
CoupledBEKinetic::computeQpResidual()
{
  Real assemble_conc = 0.0;

  for (MooseIndex(_vals) i = 0; i < _vals.size(); ++i)
    assemble_conc += _weight[i] * ((*_vals[i])[_qp] - (*_vals_old[i])[_qp]) / _dt;

  return _porosity[_qp] * _test[_i][_qp] * assemble_conc;
}
