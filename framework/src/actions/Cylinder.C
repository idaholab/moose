//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Cylinder.h"

registerMooseAction("MooseApp", Cylinder, "add_mesh_generator");
registerMooseAction("MooseApp", Cylinder, "add_positions");

InputParameters
Cylinder::validParams()
{
  InputParameters params = ComponentAction::validParams();
  params.addClassDescription("Cylindrical component.");
  params.addRequiredParam<unsigned int>(
      "dimension",
      "Dimension of the cylinder. 1 for a 1D mesh, 2 for a 2D-RZ mesh, and 3 for a 3D mesh");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius>0", "Radius of the cylinder");
  params.addRequiredRangeCheckedParam<Real>("height", "height>0", "Height of the cylinder");
  params.addRequiredParam<Point>("position", "Positional offset of the cylinder");
  params.addRequiredParam<Point>("direction", "Direction of the cylinder");

  return params;
}

Cylinder::Cylinder(const InputParameters & params) : ComponentAction(params) {}

void
Cylinder::addMeshGenerators()
{
  std::cout << "I am called correctly, now! My name is " << name() << std::endl;
}

void
Cylinder::addPositionsObject()
{
  std::cout << "add_position got registered, wow. My name is " << name() << std::endl;
}
