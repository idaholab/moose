//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementAverageMaterialProperty.h"

registerMooseObject("MooseApp", ElementAverageMaterialProperty);

InputParameters
ElementAverageMaterialProperty::validParams()
{
  InputParameters params = ElementIntegralMaterialProperty::validParams();
  params.addClassDescription("Computes the average of a material property over a volume.");
  return params;
}

ElementAverageMaterialProperty::ElementAverageMaterialProperty(const InputParameters & parameters)
  : ElementIntegralMaterialProperty(parameters), _volume(0.0)
{
}

void
ElementAverageMaterialProperty::initialize()
{
  ElementIntegralMaterialProperty::initialize();

  _volume = 0.0;
}

void
ElementAverageMaterialProperty::execute()
{
  ElementIntegralMaterialProperty::execute();

  _volume += _current_elem_volume;
}

Real
ElementAverageMaterialProperty::getValue()
{
  const Real integral = ElementIntegralMaterialProperty::getValue();

  gatherSum(_volume);

  return integral / _volume;
}

void
ElementAverageMaterialProperty::threadJoin(const UserObject & y)
{
  ElementIntegralMaterialProperty::threadJoin(y);

  const ElementAverageMaterialProperty & pps =
      static_cast<const ElementAverageMaterialProperty &>(y);
  _volume += pps._volume;
}
