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
  params.addClassDescription(
      "Computes and reports the critical time step for the explicit solver.");
  params.addParam<MaterialPropertyName>(
      "density",
      "Name of Material Property  or a constant real number defining the density of the material.");
  params.addParam<Real>("factor", 1.0, "Factor to mulitply to the critical time step.");
  return params;
}

CriticalTimeStep::CriticalTimeStep(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    GuaranteeConsumer(this),
    _material_density(getMaterialPropertyByName<Real>("density")),
    _effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness")),
    _factor(getParam<Real>("factor")),
    _critical_time(parameters.isParamValid("critical_time"))
{
}

void
CriticalTimeStep::initialSetup()
{
  if (!hasGuaranteedMaterialProperty("effective_stiffness", Guarantee::ISOTROPIC))
    paramError("CriticalTimeStep can only be used with elasticity tensor materials "
               "that guarantee isotropic tensors.");
}

void
CriticalTimeStep::initialize()
{
  _critical_time = std::numeric_limits<Real>::max();
}

void
CriticalTimeStep::execute()
{
  Real dens = _material_density[0];
  // In the above line, density is inferred only at the first quadrature point
  // of each element. Since critical time step is computed across all elements and
  // a minimum is then taken, this is okay.
  _critical_time =
      std::min(_factor * _current_elem->hmin() * std::sqrt(dens) / (_effective_stiffness[0]),
               _critical_time);
}

void
CriticalTimeStep::finalize()
{
  gatherMin(_critical_time);
}

Real
CriticalTimeStep::getValue()
{
  return _critical_time;
}

void
CriticalTimeStep::threadJoin(const UserObject & y)
{
  const CriticalTimeStep & pps = static_cast<const CriticalTimeStep &>(y);
  _critical_time += pps._critical_time;
}
