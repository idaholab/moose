//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGCylindersMeshGenerator.h"
#include "CSGBase.h"

registerMooseObject("MooseTestApp", TestCSGCylindersMeshGenerator);

InputParameters
TestCSGCylindersMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<Real>>("radii", "list of radii for concentric cylinders");
  params.addRequiredParam<Real>("height", "cylinder height");
  params.addRequiredParam<std::vector<Real>>("center", "center point of cylinder");
  params.addRequiredParam<std::string>("axis", "axis alignment");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGCylindersMeshGenerator::TestCSGCylindersMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _radii(getParam<std::vector<Real>>("radii")),
    _h(getParam<Real>("height")),
    _x0(getParam<std::vector<Real>>("center")[0]),
    _x1(getParam<std::vector<Real>>("center")[1]),
    _axis(getParam<std::string>("axis"))
{
}

std::unique_ptr<MeshBase>
TestCSGCylindersMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGCylindersMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();
  auto mg_name = this->name();

  // create the top and bottom planes
  Real a = 0;
  Real b = 0;
  Real c = 0;
  if (_axis == "x")
    a = 1;
  else if (_axis == "y")
    b = 1;
  else if (_axis == "z")
    c = 1;
  auto & pos_plane = csg_mesh->createPlaneFromCoefficients(mg_name + "_pos_plane", a, b, c, _h / 2);
  auto & neg_plane =
      csg_mesh->createPlaneFromCoefficients(mg_name + "_neg_plane", a, b, c, -1 * _h / 2);

  std::string prev_surf_name;
  for (unsigned int i = 0; i < _radii.size(); ++i)
  {
    std::string surf_name = mg_name + "_surf_cyl_" + _axis + "_" + std::to_string(i);
    auto & cyl_surf = csg_mesh->createCylinder(surf_name, _x0, _x1, _radii[i], _axis);
    CSG::CSGRegion region;
    std::string cell_name = mg_name + "_cell_cyl_" + _axis + "_" + std::to_string(i);
    if (i == 0)
      region = -cyl_surf & -pos_plane & +neg_plane;
    else
    {
      const auto & prev_surf = csg_mesh->getSurfaceByName(prev_surf_name);
      region = +prev_surf & -cyl_surf & -pos_plane & +neg_plane;
    }
    auto cell = csg_mesh->createCell(cell_name, region);
    prev_surf_name = surf_name;
  }

  return csg_mesh;
}
