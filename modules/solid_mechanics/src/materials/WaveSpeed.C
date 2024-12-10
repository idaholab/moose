//* This file is part of the MOOSE framework
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaveSpeed.h"
#include "libmesh/utility.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

registerMooseObject("SolidMechanicsApp", WaveSpeed);

InputParameters
WaveSpeed::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate the wave speed as $E / \\sqrt{\\rho}$ where $E$ is the "
                             "effective stiffness, and $\\rho$ is the material density.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");

  return params;
}

WaveSpeed::WaveSpeed(const InputParameters & parameters)
  : Material(parameters),
    _wave_speed(declareProperty<Real>("wave_speed")),
    _material_density(getMaterialPropertyByName<Real>("density")),
    _effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness"))
{
}

void
WaveSpeed::computeQpProperties()
{
  // Effective stiffness is sqrt(equivalent_youngs_modulus)
  _wave_speed[_qp] = _effective_stiffness[_qp] / std::sqrt(_material_density[_qp]);
}
