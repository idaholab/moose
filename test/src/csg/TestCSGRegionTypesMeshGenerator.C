//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGRegionTypesMeshGenerator.h"
#include "CSGBase.h"

registerMooseObject("MooseTestApp", TestCSGRegionTypesMeshGenerator);

InputParameters
TestCSGRegionTypesMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("side_length", "Side length of infinite square.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGRegionTypesMeshGenerator::TestCSGRegionTypesMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _side_length(getParam<Real>("side_length"))
{
}

std::unique_ptr<MeshBase>
TestCSGRegionTypesMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGRegionTypesMeshGenerator::generateCSG()
{
  auto csg_mesh = std::make_unique<CSG::CSGBase>();
  auto root_univ = csg_mesh->getRootUniverse();

  // initialize all surfaces to be represented
  std::vector<std::vector<Real>> plane_coeffs {
     {0, 1, 0, _side_length}, { 0, -1, 0, _side_length},
     {1, 0, 0, _side_length}, {-1,  0, 0, _side_length},
     {1, 0, 0, 0}};
  std::vector<std::string> surf_names {"plus_x", "minus_x", "plus_y", "minus_y", "zero_y"};
  CSG::CSGRegion region_left, region_right;
  for (unsigned int i = 0; i < plane_coeffs.size(); ++i)
  {
    const auto surf_name = "surf_" + surf_names[i];
    const auto plane_coeff = plane_coeffs[i];
    auto plane_ptr = csg_mesh->createPlaneFromCoefficients(surf_name, plane_coeff[0], plane_coeff[1], plane_coeff[2], plane_coeff[3]);
    auto pos_halfspace = +plane_ptr;
    auto neg_halfspace = -plane_ptr;
    if (surf_names[i] == "plus_x")
      region_right = neg_halfspace;
    else if (surf_names[i] == "minus_x")
      region_left = pos_halfspace;
    else if ((surf_names[i] == "plus_y") || (surf_names[i] == "minus_y"))
    {
      region_right &= neg_halfspace;
      region_left &= neg_halfspace;
    }
    else
    {
      region_right &= pos_halfspace;
      region_left &= neg_halfspace;
    }
  }

  CSG::CSGRegion region = region_left | region_right;
  const auto material_name = "square_material";
  csg_mesh->createCell("square_cell", material_name, region);

  CSG::CSGRegion region_complement = ~region;
  csg_mesh->createCell("void_cell", region_complement);

  return csg_mesh;
}
