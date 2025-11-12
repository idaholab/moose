//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledInjectionProductionPhysics.h"

registerPhysicsBaseTasks("PorousFlowApp", CoupledInjectionProductionPhysics);
registerMooseAction("PorousFlowApp", CoupledInjectionProductionPhysics, "add_dirac_kernel");
registerMooseAction("PorousFlowApp", CoupledInjectionProductionPhysics, "add_postprocessor");
registerMooseAction("PorousFlowApp", CoupledInjectionProductionPhysics, "add_transfer");

InputParameters
CoupledInjectionProductionPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();

  params.addRequiredParam<std::vector<Point>>("injection_points", "List of injection points [m]");
  params.addRequiredParam<std::vector<Point>>("production_points", "List of production points [m]");
  params.addParam<MultiAppName>("multi_app", "MultiApp to transfer to and from");

  params.addClassDescription(
      "Adds objects to perform hydrodynamic coupling at injection and production points.");

  return params;
}

CoupledInjectionProductionPhysics::CoupledInjectionProductionPhysics(
    const InputParameters & parameters)
  : PhysicsBase(parameters),
    _injection_points(getParam<std::vector<Point>>("injection_points")),
    _production_points(getParam<std::vector<Point>>("production_points"))
{
  _points = _injection_points;
  _points.insert(_points.end(), _production_points.begin(), _production_points.end());

  for (const auto i : index_range(_injection_points))
    _labels.push_back("inj" + std::to_string(i + 1));
  for (const auto i : index_range(_production_points))
    _labels.push_back("pro" + std::to_string(i + 1));
}

void
CoupledInjectionProductionPhysics::addDiracKernels()
{
  for (const auto i : index_range(_points))
  {
    addPPSourceDiracKernel(_points[i], "porepressure", "mass_rate_" + _labels[i]);
    addPPSourceDiracKernel(_points[i], "temperature", "energy_rate_" + _labels[i]);
  }
}

void
CoupledInjectionProductionPhysics::addPostprocessors()
{
  for (const auto i : index_range(_points))
  {
    addPointValuePostprocessor("porepressure", _points[i], "p_" + _labels[i]);
    addPointValuePostprocessor("temperature", _points[i], "T_" + _labels[i]);
    addReceiverPostprocessor("mass_rate_" + _labels[i]);
    addReceiverPostprocessor("energy_rate_" + _labels[i]);
  }
}

void
CoupledInjectionProductionPhysics::addTransfers()
{
  if (isParamValid("multi_app"))
    for (const auto i : index_range(_points))
    {
      addPostprocessorTransfer("p_" + _labels[i], false);
      addPostprocessorTransfer("T_" + _labels[i], false);
      addPostprocessorTransfer("mass_rate_" + _labels[i], true);
      addPostprocessorTransfer("energy_rate_" + _labels[i], true);
    }
}

void
CoupledInjectionProductionPhysics::addPPSourceDiracKernel(const Point & point,
                                                          const NonlinearVariableName & var,
                                                          const PostprocessorName & pp_name)
{
  const std::string class_name = "PorousFlowPointSourceFromPostprocessor";
  auto params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = var;
  params.set<Point>("point") = point;
  params.set<PostprocessorName>("mass_flux") = pp_name;
  getProblem().addDiracKernel(class_name, "dirac_" + pp_name, params);
}

void
CoupledInjectionProductionPhysics::addReceiverPostprocessor(const PostprocessorName & pp_name)
{
  const std::string class_name = "Receiver";
  auto params = _factory.getValidParams(class_name);
  getProblem().addPostprocessor(class_name, pp_name, params);
}

void
CoupledInjectionProductionPhysics::addPointValuePostprocessor(const VariableName & var,
                                                              const Point & point,
                                                              const PostprocessorName & pp_name)
{
  const std::string class_name = "PointValue";
  auto params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var;
  params.set<Point>("point") = point;
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  getProblem().addPostprocessor(class_name, pp_name, params);
}

void
CoupledInjectionProductionPhysics::addPostprocessorTransfer(const PostprocessorName & pp_name,
                                                            bool from_multi_app)
{
  const std::string class_name = "MultiAppPostprocessorTransfer";
  auto params = _factory.getValidParams(class_name);
  if (from_multi_app)
    params.set<MultiAppName>("from_multi_app") = getParam<MultiAppName>("multi_app");
  else
    params.set<MultiAppName>("to_multi_app") = getParam<MultiAppName>("multi_app");
  params.set<PostprocessorName>("from_postprocessor") = pp_name;
  params.set<PostprocessorName>("to_postprocessor") = pp_name;
  params.set<MooseEnum>("reduction_type") = "average";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  getProblem().addTransfer(class_name, pp_name + "_transfer", params);
}
