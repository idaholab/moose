//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Pressure.h"

#include "Assembly.h"
#include "Function.h"
#include "MooseError.h"
#include "FEProblemBase.h"

registerMooseObject("SolidMechanicsApp", Pressure);
registerMooseObject("SolidMechanicsApp", ADPressure);

template <bool is_ad>
InputParameters
PressureTempl<is_ad>::validParams()
{
  InputParameters params = PressureParent<is_ad>::validParams();
  params.addClassDescription("Applies a pressure on a given boundary in a given direction");
  params.addDeprecatedParam<Real>("constant",
                                  "The magnitude to use in computing the pressure",
                                  "Use 'factor' in place of 'constant'");
  params.addParam<Real>("factor", 1.0, "The magnitude to use in computing the pressure");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  params.addParam<PostprocessorName>("postprocessor",
                                     "Postprocessor that will supply the pressure value");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  return params;
}

template <bool is_ad>
PressureTempl<is_ad>::PressureTempl(const InputParameters & parameters)
  : PressureParent<is_ad>(parameters),
    _factor(parameters.isParamSetByUser("factor")     ? this->template getParam<Real>("factor")
            : parameters.isParamSetByUser("constant") ? this->template getParam<Real>("constant")
                                                      : 1.0),
    _function(this->isParamValid("function") ? &this->getFunction("function") : NULL),
    _postprocessor(
        this->isParamValid("postprocessor") ? &this->getPostprocessorValue("postprocessor") : NULL),
    _alpha(this->template getParam<Real>("alpha"))
{
  if (parameters.isParamSetByUser("factor") && parameters.isParamSetByUser("constant"))
    mooseError("Cannot set 'factor' and 'constant'.");
}

template <bool is_ad>
GenericReal<is_ad>
PressureTempl<is_ad>::computePressure() const
{
  GenericReal<is_ad> factor = _factor;

  if (_function)
    factor *= _function->value(_t + _alpha * _dt, _q_point[_qp]);

  if (_postprocessor)
    factor *= *_postprocessor;

  return factor;
}

template class PressureTempl<false>;
template class PressureTempl<true>;
