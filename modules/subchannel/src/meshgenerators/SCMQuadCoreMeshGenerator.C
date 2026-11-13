//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMQuadCoreMeshGenerator.h"
#include "MooseUtils.h"

#include <cmath>

registerMooseObject("SubChannelApp", SCMQuadCoreMeshGenerator);

InputParameters
SCMQuadCoreMeshGenerator::validParams()
{
  InputParameters params = SCMCoreMeshGeneratorBase::validParams();
  params.addClassDescription(
      "Places square-lattice subchannel assembly meshes on a rectangular core lattice. Each "
      "assembly keeps its own subdomains and boundaries; assemblies are not connected.");
  params.setDocString("inputs",
                      "The SCMQuadAssemblyMeshGenerator, SCMQuadDuctMeshGenerator, or "
                      "SCMDetailedQuadAssemblyMeshGenerator meshes that can be placed in the core");
  params.setDocString(
      "pattern",
      "Rectangular core pattern of indices into 'inputs', using the row syntax of "
      "PatternedCartesianMeshGenerator; -1 marks an empty position. All rows must have the same "
      "length. The first row is placed at +y and the entries of a row are placed from -x to +x.");
  return params;
}

SCMQuadCoreMeshGenerator::SCMQuadCoreMeshGenerator(const InputParameters & parameters)
  : SCMCoreMeshGeneratorBase(parameters)
{
  const auto n_rows = _pattern.size();
  if (n_rows == 0)
    paramError("pattern", "The pattern must contain at least one row.");

  const auto n_columns = _pattern[0].size();
  if (n_columns == 0)
    paramError("pattern", "Row 0 is empty; every row must contain at least one entry.");
  for (const auto i : index_range(_pattern))
  {
    if (_pattern[i].size() != n_columns)
      paramError("pattern",
                 "Row ",
                 i,
                 " has ",
                 _pattern[i].size(),
                 " entries; all rows of a rectangular pattern must have the ",
                 n_columns,
                 " entries of the first row.");

    // The lattice is centered on the origin
    for (const auto j : index_range(_pattern[i]))
    {
      const Real x = (Real(j) - 0.5 * Real(n_columns - 1)) * _assembly_pitch;
      const Real y = (0.5 * Real(n_rows - 1) - Real(i)) * _assembly_pitch;
      addAssembly(_pattern[i][j], Point(x, y, 0.0));
    }
  }

  if (_assemblies.empty())
    paramError("pattern", "The pattern must contain at least one assembly.");
}

void
SCMQuadCoreMeshGenerator::readAssemblySizes()
{
  for (const auto & input_name : _input_names)
  {
    if (!hasMeshProperty<Real>("duct_size_x", input_name) ||
        !hasMeshProperty<Real>("duct_size_y", input_name))
      paramError("inputs",
                 "Input '",
                 input_name,
                 "' does not provide 'duct_size_x' and 'duct_size_y' metadata. Inputs must be "
                 "SCMQuadAssemblyMeshGenerator, SCMQuadDuctMeshGenerator, or "
                 "SCMDetailedQuadAssemblyMeshGenerator objects.");
    _duct_size.emplace_back(getMeshProperty<Real>("duct_size_x", input_name),
                            getMeshProperty<Real>("duct_size_y", input_name));
  }
}

bool
SCMQuadCoreMeshGenerator::overlap(const unsigned int input_a,
                                  const unsigned int input_b,
                                  const Point & offset) const
{
  // Two axis-aligned rectangles overlap if and only if their center offset is shorter than the
  // sum of their half widths in x and shorter than the sum of their half heights in y. Ducts that
  // exactly touch are allowed; the tolerance, relative to the pitch, absorbs roundoff in the
  // lattice coordinates.
  const Real tol = libMesh::TOLERANCE * _assembly_pitch;
  const Real half_x = 0.5 * (_duct_size[input_a].first + _duct_size[input_b].first);
  const Real half_y = 0.5 * (_duct_size[input_a].second + _duct_size[input_b].second);
  return MooseUtils::absoluteFuzzyLessThan(std::abs(offset(0)), half_x, tol) &&
         MooseUtils::absoluteFuzzyLessThan(std::abs(offset(1)), half_y, tol);
}
