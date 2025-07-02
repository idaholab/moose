//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CutMeshByLevelSetGenerator.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", CutMeshByLevelSetGenerator);

InputParameters
CutMeshByLevelSetGenerator::validParams()
{
  InputParameters params = CutMeshByLevelSetGeneratorBase::validParams();

  params.addRequiredParam<std::string>(
      "level_set", "Level set used to cut the mesh as a function of x, y, and z.");
  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  params.addClassDescription(
      "This CutMeshByLevelSetGenerator object is designed to trim the input mesh by removing all "
      "the elements on outside the give level set with special processing on the elements crossed "
      "by the cutting surface to ensure a smooth cross-section. The output mesh only consists of "
      "TET4 elements.");

  return params;
}

CutMeshByLevelSetGenerator::CutMeshByLevelSetGenerator(const InputParameters & parameters)
  : CutMeshByLevelSetGeneratorBase(parameters), _level_set(getParam<std::string>("level_set"))
{
  // Create parsed function
  _func_level_set = std::make_shared<SymFunction>();
  parsedFunctionSetup(_func_level_set,
                      _level_set,
                      "x,y,z",
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"),
                      comm());

  _func_params.resize(3);
}
