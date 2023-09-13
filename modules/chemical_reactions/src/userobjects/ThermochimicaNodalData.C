//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaNodalData.h"

#include "ThermochimicaUtils.h"

registerMooseObject("ChemicalReactionsApp", ThermochimicaNodalData);

InputParameters
ThermochimicaNodalData::validParams()
{
  InputParameters params = ThermochimicaNodalBase::validParams();
  ThermochimicaUtils::addClassDescription(
      params, "Provides access to Thermochimica-calculated data at nodes.");

  return params;
}

ThermochimicaNodalData::ThermochimicaNodalData(const InputParameters & parameters)
  : ThermochimicaNodalBase(parameters)
{
  for (const auto i : make_range(_n_elements))
    _el[i] = &coupledValue("elements", i);
}

void
ThermochimicaNodalData::execute()
{
#ifdef THERMOCHIMICA_ENABLED
  std::vector<Real> element_values(_n_elements);
  for (const auto i : make_range(_n_elements))
    element_values[i] = (*_el[i])[_qp];

  auto idbg = setParamsAndRun(element_values);
  if (idbg != 0)
    mooseError("Thermochimica error ", idbg);
  else
  {
    // Save data for future reinits
    reinitDataMooseFromTc();

    // Save outputs
    setOutputNodalValues();
  }
#endif
}
