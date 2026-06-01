//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPolygonUnitMeshGenerator.h"
#include "CSGBase.h"
#include "CSGNPolygonUnit.h"

registerMooseObject("MooseTestApp", TestPolygonUnitMeshGenerator);

InputParameters
TestPolygonUnitMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("apothem", "apothem distance (center-to-flat) for the polygon.");
  params.addRequiredParam<int>("num_sides", "number of sides for for the polygon (>= 3)");
  params.addParam<bool>("expand_unit", "expand the polygon into plain surfaces");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestPolygonUnitMeshGenerator::TestPolygonUnitMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _apothem(getParam<Real>("apothem")),
    _num_sides(getParam<int>("num_sides")),
    _expand(isParamValid("expand_unit") ? getParam<bool>("expand_unit") : false)

{
}

std::unique_ptr<MeshBase>
TestPolygonUnitMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestPolygonUnitMeshGenerator::generateCSG()
{
  // name of the current mesh generator to use for naming generated objects
  auto mg_name = this->name();

  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // create an CSGNPolygonUnit for the surface
  std::unique_ptr<CSG::CSGNPolygonUnit> poly_ptr =
      std::make_unique<CSG::CSGNPolygonUnit>(mg_name + "_poly_surf", _num_sides, _apothem);
  const auto & poly = csg_obj->addEngUnit(std::move(poly_ptr));

  // create the cell with region defined by the polygon
  const auto cell_name = mg_name + "_poly_cell";
  const auto material_name = "poly_material";
  csg_obj->createCell(cell_name, material_name, -poly);

  // expand polygon unit if requested
  if (_expand)
    csg_obj->expandEngUnit(poly);

  return csg_obj;
}
