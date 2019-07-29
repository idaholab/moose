//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CriticalTimeStep.h"

registerMooseObject("TensorMechanicsApp", CriticalTimeStep);

template <>
InputParameters
validParams<CriticalTimeStep>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addClassDescription("Computes and reports the critical time step for the explicit solver.");
  params.addRequiredParam<Real>(
      "youngs_modulus", "Young's or elasticity modulus of the material.");
  params.addRequiredParam<Real>(
      "poisson_ratio", "Poisson's ratio of the material.");
  params.addRequiredParam<Real>(
      "material_density", "Density of the material.");

  return params;
}

CriticalTimeStep::CriticalTimeStep(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
  _poiss_rat(getParam<Real>("poisson_ratio")),
  _young_mod(getParam<Real>("youngs_modulus")),
  _mat_dens(getParam<Real>("material_density"))
{
}

void
CriticalTimeStep::initialize()
{
  _total_size = 0;
  _elems = 0;
}

void
CriticalTimeStep::execute()
{
  _total_size += _current_elem->hmin();
  _elems++;
}

Real
CriticalTimeStep::getValue()
{
  gatherSum(_total_size);
  gatherSum(_elems);

  Real ele_length = _total_size / _elems;
  Real ele_c = std::sqrt((_young_mod*(1-_poiss_rat))/((1+_poiss_rat)*(1-2*_poiss_rat)*_mat_dens));

  return ele_length/ele_c;
}

void
CriticalTimeStep::threadJoin(const UserObject & y)
{
  const CriticalTimeStep & pps = static_cast<const CriticalTimeStep &>(y);
  _total_size += pps._total_size;
  _elems += pps._elems;
}
