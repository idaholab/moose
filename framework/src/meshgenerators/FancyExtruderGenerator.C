//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FancyExtruderGenerator.h"

registerMooseObject("MooseApp", FancyExtruderGenerator);

template <>
InputParameters
validParams<FancyExtruderGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addClassDescription("Extrudes a 2D mesh into 3D, can have variable a variable height for "
                             "each elevation, variable number of layers within each elevation and "
                             "remap subdomain_ids within each elevation");

  params.addRequiredParam<std::vector<Real>>("heights", "The height of each elevation");

  params.addRequiredParam<std::vector<Real>>(
      "num_layers", "The number of layers for each elevation - must be num_elevations in length!");

  params.addParam<std::vector<std::vector<Real>>>(
      "subdomain_swaps",
      "For each row, every two entries are interpreted as a pair of "
      "'from' and 'to' to remap the subdomains for that elevation");

  return params;
}

FancyExtruderGenerator::FancyExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _heights(getParam<std::vector<Real>>("heights")),
    _num_layers(getParam<std::vector<Real>>("num_layers")),
    _subdomain_swaps(getParam<std::vector<std::vector<Real>>>("subdomain_swaps"))
{
}

std::unique_ptr<MeshBase>
FancyExtruderGenerator::generate()
{
  return nullptr;
}
