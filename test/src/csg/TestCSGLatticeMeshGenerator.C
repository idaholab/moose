//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSGLatticeMeshGenerator.h"
#include "MeshGenerator.h"
#include "CSGPlane.h"

registerMooseObject("MooseTestApp", TestCSGLatticeMeshGenerator);

InputParameters
TestCSGLatticeMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // input parameter that is an existing mesh generator
  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The MeshGenerators that form the components of the lattice. Order of inputs corresponds to "
      "the associated integer ID for the pattern (i.e., 0 for first input, 1 for second input, "
      "etc.)");
  params.addRequiredParam<std::string>(
      "lattice_type", "The type of lattice to create. Options are 'cartesian' and 'hexagonal'.");
  params.addRequiredParam<Real>("pitch",
                                "The pitch (flat-to-flat distance) of each lattice element.");
  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "A double-indexed array starting with the upper-left corner where the index"
      "represents the layout of inputs in the lattice.");
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

TestCSGLatticeMeshGenerator::TestCSGLatticeMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _lattice_type(getParam<std::string>("lattice_type")),
    _pitch(getParam<Real>("pitch")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern"))
{
  for (auto inp : _input_names)
  {
    _input_csgs.push_back(&getCSGBaseByName(inp));
    _mesh_ptrs.push_back(&getMeshByName(inp));
  }
}

std::unique_ptr<MeshBase>
TestCSGLatticeMeshGenerator::generate()
{
  auto null_mesh = nullptr;
  return null_mesh;
}

std::unique_ptr<CSG::CSGBase>
TestCSGLatticeMeshGenerator::generateCSG()
{
  // create a new CSGBase object to build the lattice in
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // get the name of the current mesh generator
  auto mg_name = this->name();

  // join each input CSGBase into the new CSGBase as a unique universe
  std::unordered_map<unsigned int, std::string> univ_id_names;
  for (const auto i : index_range(_input_names))
  {
    std::string join_name = _input_names[i] + "_univ";
    csg_obj->joinOtherBase(std::move(*_input_csgs[i]), join_name);
    univ_id_names[i] = join_name;
  }

  // build the universe pattern for the lattice using the input pattern
  std::vector<std::vector<std::reference_wrapper<const CSG::CSGUniverse>>> universe_pattern;
  size_t max_row_size = 0; // track max row size for later use in cell creation
  for (const auto & row : _pattern)
  {
    if (row.size() > max_row_size)
      max_row_size = row.size();

    std::vector<std::reference_wrapper<const CSG::CSGUniverse>> universe_row;
    for (const auto univ_id : row)
    {
      const auto & univ = csg_obj->getUniverseByName(univ_id_names[univ_id]);
      universe_row.push_back(univ);
    }
    universe_pattern.push_back(universe_row);
  }

  // create the lattice based on the specified type
  std::string lat_name = mg_name + "_lattice";
  if (_lattice_type == "cartesian")
  {
    csg_obj->createCartesianLattice(lat_name, _pitch, universe_pattern);
  }
  else if (_lattice_type == "hexagonal")
  {
    csg_obj->createHexagonalLattice(lat_name, _pitch, universe_pattern);
  }

  // create a cell and insert lattice as the fill
  // make the cell a cube region where x dimension is pitch * max number of elements in a row
  // and y dimension is pitch * number of rows
  Real x_size = _pitch * max_row_size;
  Real y_size = _pitch * _pattern.size();
  // surfaces for regions for cell
  std::unique_ptr<CSG::CSGSurface> surf_xmax =
      std::make_unique<CSG::CSGPlane>(mg_name + "_surf_plus_x", 1.0, 0.0, 0.0, x_size / 2.0);
  std::unique_ptr<CSG::CSGSurface> surf_xmin =
      std::make_unique<CSG::CSGPlane>(mg_name + "_surf_minus_x", 1.0, 0.0, 0.0, -x_size / 2.0);
  std::unique_ptr<CSG::CSGSurface> surf_ymax =
      std::make_unique<CSG::CSGPlane>(mg_name + "_surf_plus_y", 0.0, 1.0, 0.0, y_size / 2.0);
  std::unique_ptr<CSG::CSGSurface> surf_ymin =
      std::make_unique<CSG::CSGPlane>(mg_name + "_surf_minus_y", 0.0, 1.0, 0.0, -y_size / 2.0);
  auto & xmax = csg_obj->addSurface(std::move(surf_xmax));
  auto & xmin = csg_obj->addSurface(std::move(surf_xmin));
  auto & ymax = csg_obj->addSurface(std::move(surf_ymax));
  auto & ymin = csg_obj->addSurface(std::move(surf_ymin));
  // cell region
  auto region = (-xmax & +xmin & -ymax & +ymin);
  // create the cell with the lattice as the fill
  csg_obj->createCell(mg_name + "_lattice_cell", csg_obj->getLatticeByName(lat_name), region);

  return csg_obj;
}
