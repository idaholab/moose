//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriCoreMeshGenerator.h"
#include "MooseUtils.h"

#include <array>
#include <cmath>

registerMooseObject("SubChannelApp", SCMTriCoreMeshGenerator);

InputParameters
SCMTriCoreMeshGenerator::validParams()
{
  InputParameters params = SCMCoreMeshGeneratorBase::validParams();
  params.addClassDescription(
      "Places triangular-lattice subchannel assembly meshes on a hexagonal core lattice. Each "
      "assembly keeps its own subdomains and boundaries; assemblies are not connected.");
  params.setDocString("inputs",
                      "The SCMTriAssemblyMeshGenerator, SCMTriDuctMeshGenerator, or "
                      "SCMDetailedTriAssemblyMeshGenerator meshes that can be placed in the core");
  params.setDocString(
      "pattern",
      "Hexagonal core pattern of indices into 'inputs', using the row syntax of "
      "PatternedHexMeshGenerator; -1 marks an empty position. Row i is placed as the i-th column "
      "from -x, and the entries of a row are placed from -y to +y.");
  return params;
}

SCMTriCoreMeshGenerator::SCMTriCoreMeshGenerator(const InputParameters & parameters)
  : SCMCoreMeshGeneratorBase(parameters)
{
  const auto n_rows = _pattern.size();
  if (n_rows == 0)
    paramError("pattern", "The pattern must contain at least one row.");
  if (n_rows % 2 == 0)
    paramError("pattern", "A hexagonal pattern must have an odd number of rows.");

  // Number of positions along each side of the hexagonal pattern
  const auto n_side = (n_rows + 1) / 2;
  for (const auto i : index_range(_pattern))
  {
    const auto row_length = n_side + std::min(i, n_rows - 1 - i);
    if (_pattern[i].size() != row_length)
      paramError("pattern",
                 "Row ",
                 i,
                 " has ",
                 _pattern[i].size(),
                 " entries; a hexagonal pattern with ",
                 n_rows,
                 " rows requires ",
                 row_length,
                 ".");

    for (const auto j : index_range(_pattern[i]))
    {
      // The SCM assemblies have a duct corner on the +x axis, so their flat normals point at 30,
      // 90 and 150 degrees. Placing rows as columns, with neighboring columns offset by half a
      // pitch, puts every pair of neighboring positions along one of those normals.
      const Real x = (Real(i) - Real(n_side - 1)) * _assembly_pitch * std::sqrt(3.0) / 2.0;
      const Real y = (Real(j) - 0.5 * Real(row_length - 1)) * _assembly_pitch;
      addAssembly(_pattern[i][j], Point(x, y, 0.0));
    }
  }

  if (_assemblies.empty())
    paramError("pattern", "The pattern must contain at least one assembly.");
}

void
SCMTriCoreMeshGenerator::readAssemblySizes()
{
  for (const auto & input_name : _input_names)
  {
    if (!hasMeshProperty<Real>("flat_to_flat", input_name))
      paramError("inputs",
                 "Input '",
                 input_name,
                 "' does not provide 'flat_to_flat' metadata. Inputs must be "
                 "SCMTriAssemblyMeshGenerator, SCMTriDuctMeshGenerator, or "
                 "SCMDetailedTriAssemblyMeshGenerator objects.");
    _flat_to_flat.push_back(getMeshProperty<Real>("flat_to_flat", input_name));
  }
}

bool
SCMTriCoreMeshGenerator::overlap(const unsigned int input_a,
                                 const unsigned int input_b,
                                 const Point & offset) const
{
  // The ducts are hexagons with the same orientation, and the set of center offsets at which two
  // of them overlap is a hexagon of that orientation whose apothem is the sum of their apothems.
  // They therefore overlap if and only if the center offset projected on every flat normal is
  // shorter than the sum of the apothems.
  const std::array<Point, 3> flat_normals = {Point(std::sqrt(3.0) / 2.0, 0.5, 0.0),
                                             Point(0.0, 1.0, 0.0),
                                             Point(-std::sqrt(3.0) / 2.0, 0.5, 0.0)};
  // Ducts that exactly touch (assembly_pitch equal to the flat-to-flat size) are allowed; the
  // tolerance, relative to the pitch, absorbs roundoff in the lattice coordinates.
  const Real tol = libMesh::TOLERANCE * _assembly_pitch;
  const Real sum_apothems = 0.5 * (_flat_to_flat[input_a] + _flat_to_flat[input_b]);
  for (const auto & normal : flat_normals)
    if (!MooseUtils::absoluteFuzzyLessThan(std::abs(offset * normal), sum_apothems, tol))
      return false;
  return true;
}
