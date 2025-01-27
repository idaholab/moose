//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CutMeshByPlaneGenerator.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", CutMeshByPlaneGenerator);

InputParameters
CutMeshByPlaneGenerator::validParams()
{
  InputParameters params = CutMeshByLevelSetGeneratorBase::validParams();

  params.addRequiredParam<Point>("plane_point", "A point on the plane.");
  params.addRequiredParam<Point>("plane_normal", "The normal vector of the plane.");

  params.addClassDescription(
      "This CutMeshByPlaneGenerator object is designed to trim the input mesh by removing all the "
      "elements on one side of a given plane with special processing on the elements crossed by "
      "the cutting plane to ensure a smooth cross-section. The output mesh only consists of TET4 "
      "elements.");

  return params;
}

CutMeshByPlaneGenerator::CutMeshByPlaneGenerator(const InputParameters & parameters)
  : CutMeshByLevelSetGeneratorBase(parameters),
    _plane_point(getParam<Point>("plane_point")),
    _plane_normal(getParam<Point>("plane_normal").unit())
{
  // Translate the plane point and plane normal to the form of the level set function
  _func_level_set = std::make_shared<SymFunction>();
  // set FParser internal feature flags
  setParserFeatureFlags(_func_level_set);
  // The plane is (x - x0) * n_x + (y - y0) * n_y + (z - z0) * n_z = 0
  std::stringstream level_set_ss;
  // Let's be conservative about precision here
  level_set_ss << std::fixed << std::setprecision(15) << _plane_normal(0) << "*(x-"
               << _plane_point(0) << ") + " << _plane_normal(1) << "*(y-" << _plane_point(1)
               << ") + " << _plane_normal(2) << "*(z-" << _plane_point(2) << ")";

  // VERY unlikely to reach this point because we know what we are doing
  // But just in case
  if (_func_level_set->Parse(level_set_ss.str(), "x,y,z") >= 0)
    mooseError("The given plane_point and plane_normal lead to invalid level set.\n",
               _func_level_set,
               "\nin CutMeshByPlaneGenerator ",
               name(),
               ".\n",
               _func_level_set->ErrorMsg());
  _func_params.resize(3);
}
