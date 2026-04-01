//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetMeshingHelper.h"

InputParameters
LevelSetMeshingHelper::validParams()
{
  InputParameters params = FunctionParserUtils<false>::validParams();
  params.renameParameterGroup("Parsed expression advanced", "Level set shape parsed expression");

  params.addParam<std::string>(
      "level_set",
      "Level set used to achieve more accurate reverse projection compared to interpolation.");
  params.addParam<unsigned int>(
      "max_level_set_correction_iterations",
      3,
      "Maximum number of iterations to correct the nodes based on the level set function.");
  params.addParamNamesToGroup("level_set max_level_set_correction_iterations",
                              "Level set shape correction");

  return params;
}

LevelSetMeshingHelper::LevelSetMeshingHelper(const InputParameters & parameters)
  : FunctionParserUtils<false>(parameters),
    _max_level_set_correction_iterations(
        parameters.get<unsigned int>("max_level_set_correction_iterations"))
{
  if (parameters.isParamValid("level_set"))
  {
    _func_level_set = std::make_shared<SymFunction>();
    // set FParser internal feature flags
    setParserFeatureFlags(_func_level_set);
    if (parameters.isParamValid("constant_names") &&
        parameters.isParamValid("constant_expressions"))
      addFParserConstants(_func_level_set,
                          parameters.get<std::vector<std::string>>("constant_names"),
                          parameters.get<std::vector<std::string>>("constant_expressions"));
    if (_func_level_set->Parse(parameters.get<std::string>("level_set"), "x,y,z") >= 0)
      mooseError(
          "Invalid function f(x,y,z)\n", _func_level_set, ".\n", _func_level_set->ErrorMsg());

    _func_params.resize(3);
  }
}

Real
LevelSetMeshingHelper::levelSetEvaluator(const Point & point)
{
  return evaluate(_func_level_set, std::vector<Real>({point(0), point(1), point(2), 0}));
}

void
LevelSetMeshingHelper::levelSetCorrection(Node & node)
{
  // Based on the given level set, we try to move the node in its normal direction
  const Real diff = libMesh::TOLERANCE * 10.0; // A small value to perturb the node
  const Real original_eval = levelSetEvaluator(node);
  const Real xp_eval = levelSetEvaluator(node + Point(diff, 0.0, 0.0));
  const Real yp_eval = levelSetEvaluator(node + Point(0.0, diff, 0.0));
  const Real zp_eval = levelSetEvaluator(node + Point(0.0, 0.0, diff));
  const Real xm_eval = levelSetEvaluator(node - Point(diff, 0.0, 0.0));
  const Real ym_eval = levelSetEvaluator(node - Point(0.0, diff, 0.0));
  const Real zm_eval = levelSetEvaluator(node - Point(0.0, 0.0, diff));
  const Point grad = Point((xp_eval - xm_eval) / (2.0 * diff),
                           (yp_eval - ym_eval) / (2.0 * diff),
                           (zp_eval - zm_eval) / (2.0 * diff));
  const Real xyz_diff = -original_eval / grad.contract(grad);
  node(0) += xyz_diff * grad(0);
  node(1) += xyz_diff * grad(1);
  node(2) += xyz_diff * grad(2);
}
