//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureConvergence.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureConvergence);

InputParameters
HeatStructureConvergence::validParams()
{
  InputParameters params = MultiPostprocessorConvergence::validParams();

  params.addClassDescription("Assesses convergence of a HeatStructure component.");

  params.addRequiredParam<std::vector<SubdomainName>>("blocks", "Heat structure blocks");

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "T_rel_step", "Temperature relative step post-processor for each block");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "res", "Normalized residual norm post-processor for each block [1/s]");

  params.addRequiredParam<Real>("T_rel_step_tol", "Relative step tolerance for temperature");
  params.addRequiredParam<Real>("res_tol", "Normalized residual tolerance [1/s]");

  return params;
}

HeatStructureConvergence::HeatStructureConvergence(const InputParameters & parameters)
  : MultiPostprocessorConvergence(parameters),
    _blocks(getParam<std::vector<SubdomainName>>("blocks")),
    _T_rel_step_tol(getParam<Real>("T_rel_step_tol")),
    _res_tol(getParam<Real>("res_tol")),
    _n_blocks(_blocks.size())
{
  const auto T_rel_step = getParam<std::vector<PostprocessorName>>("T_rel_step");
  const auto res = getParam<std::vector<PostprocessorName>>("res");
  if ((T_rel_step.size() != _n_blocks) || (res.size() != _n_blocks))
  {
    mooseAssert(false,
                "The parameters 'blocks', 'T_rel_step', and 'res' must all be the same size.");
  }

  for (const auto i : make_range(_n_blocks))
  {
    _T_rel_step.push_back(&getPostprocessorValueByName(T_rel_step[i]));
    _res.push_back(&getPostprocessorValueByName(res[i]));
  }
}

std::vector<std::tuple<std::string, Real, Real>>
HeatStructureConvergence::getDescriptionErrorToleranceTuples() const
{
  std::vector<std::tuple<std::string, Real, Real>> tuples;
  for (const auto i : make_range(_n_blocks))
    tuples.push_back({"T step (" + _blocks[i] + ")", *(_T_rel_step[i]), _T_rel_step_tol});
  for (const auto i : make_range(_n_blocks))
    tuples.push_back({"residual (" + _blocks[i] + ")", *(_res[i]), _res_tol});
  return tuples;
}
