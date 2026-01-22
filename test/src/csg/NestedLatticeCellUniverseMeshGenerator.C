//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedLatticeCellUniverseMeshGenerator.h"
#include "CSGBase.h"
#include "CSGSphere.h"
#include "CSGCartesianLattice.h"

registerMooseObject("MooseTestApp", NestedLatticeCellUniverseMeshGenerator);

InputParameters
NestedLatticeCellUniverseMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Real>("inner_radius", "Inner radius of sphere.");
  params.addRequiredParam<Real>("outer_radius", "Outer radius of sphere.");
  params.addRequiredParam<Real>("lattice_radius", "Outer radius of cell with lattice fill.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

NestedLatticeCellUniverseMeshGenerator::NestedLatticeCellUniverseMeshGenerator(
    const InputParameters & params)
  : MeshGenerator(params),
    _inner_rad(getParam<Real>("inner_radius")),
    _outer_rad(getParam<Real>("outer_radius")),
    _lattice_rad(getParam<Real>("lattice_radius"))
{
}

std::unique_ptr<MeshBase>
NestedLatticeCellUniverseMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
NestedLatticeCellUniverseMeshGenerator::generateCSG()
{
  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // create universe based on inner sphere
  auto & nested_inner_univ = csg_obj->createUniverse("nested_inner_univ");
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_inner =
      std::make_unique<CSG::CSGSphere>("inner_surf", _inner_rad);
  auto & csg_sphere_inner = csg_obj->addSurface(std::move(sphere_ptr_inner));
  csg_obj->createCell("cell_inner1", "mat1", -csg_sphere_inner, &nested_inner_univ);
  csg_obj->createCell("cell_inner2", "mat2", +csg_sphere_inner, &nested_inner_univ);

  // create cell with universe fill
  auto & nested_outer_univ = csg_obj->createUniverse("nested_outer_univ");
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_outer =
      std::make_unique<CSG::CSGSphere>("outer_surf", _outer_rad);
  auto & csg_sphere_outer = csg_obj->addSurface(std::move(sphere_ptr_outer));
  csg_obj->createCell("cell_univ_fill", nested_inner_univ, -csg_sphere_outer, &nested_outer_univ);
  csg_obj->createCell("cell_outer", "mat3", +csg_sphere_outer, &nested_outer_univ);

  // create cell with 2x2 Cartesian lattice fill
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_lattice =
      std::make_unique<CSG::CSGSphere>("lattice_surf", _lattice_rad);
  auto & csg_sphere_lattice = csg_obj->addSurface(std::move(sphere_ptr_lattice));
  std::vector<std::vector<std::reference_wrapper<const CSG::CSGUniverse>>> lat_univs = {
      {nested_outer_univ, nested_outer_univ}, {nested_outer_univ, nested_outer_univ}};
  auto & lattice_outer = csg_obj->createUniverse("lattice_outer");
  std::unique_ptr<CSG::CSGCartesianLattice> lat_ptr =
      std::make_unique<CSG::CSGCartesianLattice>("cartesian_lat", 10.0, lat_univs);
  const auto & lattice = csg_obj->addLattice<CSG::CSGCartesianLattice>(std::move(lat_ptr));
  csg_obj->setLatticeOuter(lattice, lattice_outer);

  csg_obj->createCell("cell_lat_fill", lattice, -csg_sphere_lattice);
  csg_obj->createCell("cell_void", +csg_sphere_lattice);

  return csg_obj;
}
