//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActivateElementsCoupled.h"

registerMooseObject("MooseApp", ActivateElementsCoupled);

InputParameters
ActivateElementsCoupled::validParams()
{
  InputParameters params = ActivateElementsUserObjectBase::validParams();

  params.addRequiredParam<Real>("activate_value", "The value above which to activate the element");
  params.addRequiredCoupledVar(
      "coupled_var",
      "The variable value will be used to decide wether an element whould be activated.");
  params.addParam<MooseEnum>("activate_type",
                             MooseEnum("below equal above", "above"),
                             "Activate element when below or above the activate_value");
  return params;
}

ActivateElementsCoupled::ActivateElementsCoupled(const InputParameters & parameters)
  : ActivateElementsUserObjectBase(parameters),
    _coupled_var(coupledValue("coupled_var")),
    _activate_value(
        declareRestartableData<Real>("activate_value", getParam<Real>("activate_value"))),
    _activate_type(getParam<MooseEnum>("activate_type").getEnum<ActivateType>())
{
}

bool
ActivateElementsCoupled::isElementActivated()
{
  bool is_activated = false;
  Real avg_val = 0.0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    avg_val += _coupled_var[qp];
  avg_val /= _qrule->n_points();

  switch (_activate_type)
  {
    case ActivateType::BELOW:
      is_activated = (avg_val < _activate_value);
      break;

    case ActivateType::EQUAL:
      is_activated = MooseUtils::absoluteFuzzyEqual(avg_val - _activate_value, 0.0);
      break;

    case ActivateType::ABOVE:
      is_activated = (avg_val > _activate_value);
      break;
  }

  return is_activated;
}
