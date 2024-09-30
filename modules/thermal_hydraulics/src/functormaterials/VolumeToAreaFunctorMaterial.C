//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeToAreaFunctorMaterial.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", VolumeToAreaFunctorMaterial);

InputParameters
VolumeToAreaFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Creates a conversion factor for integrating surface term over the volume of a 1D element");

  params.addRequiredParam<MooseFunctorName>("functor_name",
                                            "Name of the conversion factor functor");
  params.addRequiredParam<MooseFunctorName>("area",
                                            "Name of the functor providing the target area");
  params.addParam<MooseFunctorName>("coef", "1", "Coefficient multiplying the conversion factor");

  return params;
}

VolumeToAreaFunctorMaterial::VolumeToAreaFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _area(getFunctor<ADReal>("area")),
    _coef(getFunctor<ADReal>("coef"))
{
  addFunctorProperty<ADReal>(
      getParam<MooseFunctorName>("functor_name"),
      [this](const auto & r, const auto & t) -> ADReal
      {
        Real volume = 0;

        if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
          volume = r.elem->volume();
        else if constexpr (std::is_same_v<const Moose::ElemQpArg &, decltype(r)>)
          volume = r.elem->volume();
        else if constexpr (std::is_same_v<const Moose::ElemPointArg &, decltype(r)>)
          volume = r.elem->volume();
        else
          mooseError("Area to volume conversion is only implemented for element-based functor"
                     "arguments, not for ",
                     MooseUtils::prettyCppType(&r),
                     ". Please contact a MOOSE developer or implement it yourself.");

        return _coef(r, t) * _area(r, t) / volume;
      });
}
