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
#include "CSGPlane.h"
#include "CSGXCylinder.h"
#include "CSGYCylinder.h"
#include "CSGZCylinder.h"

registerMooseObject("MooseTestApp", TestCSGCylindersMeshGenerator);

InputParameters
TestCSGCylindersMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<Real>>("radii", "list of radii for concentric cylinders");
  params.addRequiredParam<Real>("height", "cylinder height");
  params.addRequiredParam<std::vector<Real>>("center", "center point of cylinder");
  MooseEnum axis("x y z", "x");
  params.addParam<MooseEnum>("axis", axis, "axis alignment");
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
    _axis(getParam<MooseEnum>("axis"))
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
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto mg_name = this->name();

  // make the top and bottom planes
  Real a = 0;
  Real b = 0;
  Real c = 0;
  if (_axis == "x")
    a = 1;
  else if (_axis == "y")
    b = 1;
  else if (_axis == "z")
    c = 1;
  std::unique_ptr<CSG::CSGSurface> pos_plane_ptr =
      std::make_unique<CSG::CSGPlane>(mg_name + "_pos_plane", a, b, c, _h / 2);
  auto & pos_plane = csg_obj->addSurface(std::move(pos_plane_ptr));
  std::unique_ptr<CSG::CSGSurface> neg_plane_ptr =
      std::make_unique<CSG::CSGPlane>(mg_name + "_neg_plane", a, b, c, -1 * _h / 2);
  auto & neg_plane = csg_obj->addSurface(std::move(neg_plane_ptr));

  std::string prev_surf_name;
  for (unsigned int i = 0; i < _radii.size(); ++i)
  {
    // create a cylinder surface of the specified orientation (x, y, or z)
    std::string surf_name = mg_name + "_surf_cyl_" + _axis + "_" + std::to_string(i);
    std::unique_ptr<CSG::CSGSurface> cyl_ptr;
    Point cyl_origin;
    if (_axis == "x")
    {
      cyl_ptr = std::make_unique<CSG::CSGXCylinder>(surf_name, _x0, _x1, _radii[i]);
      cyl_origin = Point(0, _x0, _x1);
    }
    else if (_axis == "y")
    {
      cyl_ptr = std::make_unique<CSG::CSGYCylinder>(surf_name, _x0, _x1, _radii[i]);
      cyl_origin = Point(_x0, 0, _x1);
    }
    else if (_axis == "z")
    {
      cyl_ptr = std::make_unique<CSG::CSGZCylinder>(surf_name, _x0, _x1, _radii[i]);
      cyl_origin = Point(_x0, _x1, 0);
    }

    auto & cyl_surf = csg_obj->addSurface(std::move(cyl_ptr));
    auto cyl_halfspace = cyl_surf.getHalfspaceFromPoint(cyl_origin);
    auto cyl_region = CSG::CSGRegion(cyl_surf, cyl_halfspace);

    CSG::CSGRegion full_region;
    std::string cell_name = mg_name + "_cell_cyl_" + _axis + "_" + std::to_string(i);
    if (i == 0)
      full_region = cyl_region & -pos_plane & +neg_plane;
    else
    {
      const auto & prev_surf = csg_obj->getSurfaceByName(prev_surf_name);
      full_region = +prev_surf & cyl_region & -pos_plane & +neg_plane;
    }
    auto cell = csg_obj->createCell(cell_name, full_region);
    prev_surf_name = surf_name;
  }

  return csg_obj;
}
