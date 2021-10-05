//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CurrentDensity.h"

registerMooseObject("ElectromagneticsApp", CurrentDensity);
registerMooseObject("ElectromagneticsApp", ADCurrentDensity);

template <bool is_ad>
InputParameters
CurrentDensityTempl<is_ad>::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the current density vector field (in A/m^2) when given electrostatic "
      "potential (electrostatic = true, default) or electric field.");
  params.addParam<bool>("electrostatic",
                        true,
                        "Whether the electric field is based on electrostatic potential or is "
                        "fully electromagnetic (default = TRUE)");
  params.addCoupledVar("potential", "Electrostatic potential variable");
  params.addCoupledVar("electric_field", "Electric field variable (electromagnetic)");
  return params;
}

template <bool is_ad>
CurrentDensityTempl<is_ad>::CurrentDensityTempl(const InputParameters & parameters)
  : VectorAuxKernel(parameters),

    _is_es(getParam<bool>("electrostatic")),
    _grad_potential(isParamValid("potential") ? coupledGradient("potential") : _grad_zero),
    _electric_field(isParamValid("electric_field") ? coupledVectorValue("electric_field")
                                                   : _vector_zero),

    _conductivity(getGenericMaterialProperty<Real, is_ad>("electrical_conductivity"))
{
  if (_is_es && !isParamValid("potential") && isParamValid("electric_field"))
  {
    mooseError(
        "In ",
        name(),
        ", an electric field vector variable has been provided when `electrostatic = TRUE`. Please "
        "either provide an electrostatic potential variable only or set `electrostatic = FALSE`!");
  }
  else if (!_is_es && isParamValid("potential") && !isParamValid("electric_field"))
  {
    mooseError("In ",
               name(),
               ", an electrostatic potential variable has been provided when `electrostatic = "
               "FALSE`. Please either provide an electric field vector variable only or set "
               "`electrostatic = TRUE`!");
  }
  else if (isParamValid("potential") && isParamValid("electric_field"))
  {
    mooseError("In ",
               name(),
               ", both electrostatic potential and electric field variables have been provided. "
               "Please only provide one or the other!");
  }
}

template <bool is_ad>
RealVectorValue
CurrentDensityTempl<is_ad>::computeValue()
{
  if (_is_es)
    return MetaPhysicL::raw_value(_conductivity[_qp]) * -_grad_potential[_qp];
  else
    return MetaPhysicL::raw_value(_conductivity[_qp]) * _electric_field[_qp];
}

template class CurrentDensityTempl<false>;
template class CurrentDensityTempl<true>;
