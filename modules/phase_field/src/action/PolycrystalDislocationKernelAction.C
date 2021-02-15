/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#include "PolycrystalDislocationKernelAction.h"
#include "Factory.h"
#include "FEProblemBase.h"

registerMooseAction("MarmotApp", PolycrystalDislocationKernelAction, "add_kernel");

template <>
InputParameters
validParams<PolycrystalDislocationKernelAction>()
{
  InputParameters params = validParams<PolycrystalKernelAction>();
  params.addRequiredParam<std::string>(
      "grain_tracker",
      "the grain tracker user object to get values from for dislocation densities");

  params.addClassDescription(
      "set up ACPolycrystalDislocationEnergy + kernels from PolycrystalKernel");

  return params;
}

PolycrystalDislocationKernelAction::PolycrystalDislocationKernelAction(const InputParameters & params)
  : PolycrystalKernelAction(params), _grain_tracker_name(getParam<std::string>("grain_tracker"))
{
}

void
PolycrystalDislocationKernelAction::act()
{
  // Set up all the other kernels needed
  PolycrystalKernelAction::act();

  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Create the variable names
    std::string var_name = _var_name_base + Moose::stringify(op);
    std::vector<VariableName> v;
    v.resize(_op_num - 1);

    unsigned int ind = 0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != op)
        v[ind++] = _var_name_base + Moose::stringify(j);

    // Set up dislocation energy contribution kernels
    InputParameters params = _factory.getValidParams("ACPolycrystalDislocationEnergy");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<std::vector<VariableName>>("v") = v;
    params.set<unsigned int>("op_index") = op;
    params.set<UserObjectName>("grain_tracker") = _grain_tracker_name;

    params.applyParameters(parameters());

    std::string kernel_name = "ACGrGrDisloc_" + var_name;
    _problem->addKernel("ACPolycrystalDislocationEnergy", kernel_name, params);
  }
}
