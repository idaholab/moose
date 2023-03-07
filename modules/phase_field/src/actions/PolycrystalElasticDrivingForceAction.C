//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalElasticDrivingForceAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("PhaseFieldApp", PolycrystalElasticDrivingForceAction, "add_kernel");

InputParameters
PolycrystalElasticDrivingForceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action that adds the elastic driving force for each order parameter");
  params.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

PolycrystalElasticDrivingForceAction::PolycrystalElasticDrivingForceAction(
    const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor_name(_base_name + "elasticity_tensor")
{
}

void
PolycrystalElasticDrivingForceAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalElasticDrivingForceAction Object\n";
  Moose::err << "var name base:" << _var_name_base << std::flush;
#endif

  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Create variable name
    std::string var_name = _var_name_base + Moose::stringify(op);

    // Create Stiffness derivative name
    MaterialPropertyName D_stiff_name =
        derivativePropertyNameFirst(_elasticity_tensor_name, var_name);

    // Set name of kernel being created
    std::string kernel_type = "ACGrGrElasticDrivingForce";

    // Set the actual parameters for the kernel
    InputParameters poly_params = _factory.getValidParams(kernel_type);
    poly_params.set<NonlinearVariableName>("variable") = var_name;
    poly_params.set<MaterialPropertyName>("D_tensor_name") = D_stiff_name;
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    std::string kernel_name = "AC_ElasticDrivingForce_" + var_name;

    // Create kernel
    _problem->addKernel(kernel_type, kernel_name, poly_params);
  }
}
