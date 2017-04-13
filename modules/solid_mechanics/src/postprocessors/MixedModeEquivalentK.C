/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MixedModeEquivalentK.h"

template <>
InputParameters
validParams<MixedModeEquivalentK>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("KI_name", "The name of the KI postprocessor");
  params.addRequiredParam<PostprocessorName>("KII_name", "The name of the KII postprocessor");
  params.addRequiredParam<PostprocessorName>("KIII_name", "The name of the KIII postprocessor");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  return params;
}

MixedModeEquivalentK::MixedModeEquivalentK(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _ki_value(getPostprocessorValue("KI_name")),
    _kii_value(getPostprocessorValue("KII_name")),
    _kiii_value(getPostprocessorValue("KIII_name")),
    _poissons_ratio(getParam<Real>("poissons_ratio"))
{
}

Real
MixedModeEquivalentK::getValue()
{
  return std::sqrt(_ki_value * _ki_value + _kii_value * _kii_value +
                   1 / (1 - _poissons_ratio) * _kiii_value * _kiii_value);
}
