//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PolyChaosData.h"

registerMooseObject("StochasticToolsApp", PolyChaosData);

defineLegacyParams(PolyChaosData);

InputParameters
PolyChaosData::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tool for extracting data from polynomial chaos user object and "
                             "storing in VectorPostprocessor vectors.");
  params.addRequiredParam<UserObjectName>("pc_name", "Name of PolynomialChaos.");
  return params;
}

PolyChaosData::PolyChaosData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _pc_uo(getUserObject<PolynomialChaos>("pc_name")),
    _coeff(_pc_uo.getCoefficients()),
    _coeff_vector(declareVector("coefficients")),
    _initialized(false)
{
}

void
PolyChaosData::initialize()
{
  // This needs to be put in initialize since the user object doesn't know the
  // number of parameters until after initialSetup.
  if (!_initialized)
    for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
      _order_vector.push_back(&declareVector("order_p" + std::to_string(d)));

  _initialized = true;
}

void
PolyChaosData::execute()
{
  _coeff_vector.clear();
  _coeff_vector = _coeff;

  for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
  {
    _order_vector[d]->resize(_pc_uo.getNumberofCoefficients());
    for (unsigned int i = 0; i < _pc_uo.getNumberofCoefficients(); ++i)
      (*_order_vector[d])[i] = _pc_uo.getPolynomialOrder(d, i);
  }
}
