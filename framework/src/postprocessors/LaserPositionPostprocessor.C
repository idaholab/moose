//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaserPositionPostprocessor.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MathUtils.h"
#include "TransientBase.h"
#include "Restartable.h"
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", LaserPositionPostprocessor);

InputParameters
LaserPositionPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>("speed","Lift coeff");

  params.addClassDescription("Blabla.");

  return params;
}

LaserPositionPostprocessor::LaserPositionPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _speed(getPostprocessorValue("speed")),
    _current_arclength(0.0),
    _delta_arclength(0.0)
{
}

Real
LaserPositionPostprocessor::getValue() const
{
  return _current_arclength;
}

void
LaserPositionPostprocessor::execute()
{
  _delta_arclength = _speed * _dt;
  _current_arclength += _delta_arclength;
}
