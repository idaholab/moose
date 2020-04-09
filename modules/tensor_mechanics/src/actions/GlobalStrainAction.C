//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalStrainAction.h"

#include "Conversion.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("TensorMechanicsApp", GlobalStrainAction, "add_user_object");

registerMooseAction("TensorMechanicsApp", GlobalStrainAction, "add_scalar_kernel");

registerMooseAction("TensorMechanicsApp", GlobalStrainAction, "add_material");

registerMooseAction("TensorMechanicsApp", GlobalStrainAction, "add_aux_variable");

registerMooseAction("TensorMechanicsApp", GlobalStrainAction, "add_aux_kernel");

InputParameters
GlobalStrainAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up the GlobalStrainAction environment");
  params.addRequiredParam<VariableName>("scalar_global_strain",
                                        "Scalar variable for global strain");
  params.addParam<std::vector<VariableName>>("displacements", "The displacement variables");
  params.addParam<std::vector<AuxVariableName>>(
      "auxiliary_displacements",
      "The auxliary displacement variables to be calculated from scalar variables");
  params.addParam<std::vector<AuxVariableName>>(
      "global_displacements",
      "The global displacement variables to be calculated from scalar variables");
  params.addParam<std::vector<Real>>("applied_stress_tensor",
                                     "Vector of values defining the constant applied stress "
                                     "to add, in order 11, 22, 33, 23, 13, 12");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this variable lives");

  return params;
}

GlobalStrainAction::GlobalStrainAction(const InputParameters & params)
  : Action(params),
    _disp(getParam<std::vector<VariableName>>("displacements")),
    _aux_disp(getParam<std::vector<AuxVariableName>>("auxiliary_displacements")),
    _global_disp(getParam<std::vector<AuxVariableName>>("global_displacements")),
    _block_names(getParam<std::vector<SubdomainName>>("block")),
    _block_ids()
{
  if (_aux_disp.size() != _disp.size())
    mooseError("Number of auxiliary displacement variables should be equal to the number of "
               "nonlinear displacement variables, i.e., ",
               _disp.size());
}

void
GlobalStrainAction::act()
{
  // get subdomain IDs
  for (auto & name : _block_names)
    _block_ids.insert(_problem->mesh().getSubdomainID(name));

  // user object name
  const std::string uo_name = _name + "_GlobalStrainUserObject";

  //
  // Add user object
  //
  if (_current_task == "add_user_object")
  {
    std::string uo_type = "GlobalStrainUserObject";
    InputParameters params = _factory.getValidParams(uo_type);
    params.applyParameters(parameters());
    params.set<bool>("use_displaced_mesh") = false;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

    _problem->addUserObject(uo_type, uo_name, params);
  }

  //
  // Add scalar kernel
  //
  else if (_current_task == "add_scalar_kernel")
  {
    std::string sk_type = "GlobalStrain";
    InputParameters params = _factory.getValidParams(sk_type);
    params.applyParameters(parameters());
    params.set<bool>("use_displaced_mesh") = false;
    params.set<NonlinearVariableName>("variable") = getParam<VariableName>("scalar_global_strain");
    params.set<UserObjectName>("global_strain_uo") = uo_name;

    _problem->addScalarKernel(sk_type, _name + "_GlobalStrain", params);
  }

  //
  // Add ComputeGlobalStrain material
  //
  else if (_current_task == "add_material")
  {
    std::string mat_type = "ComputeGlobalStrain";
    InputParameters params = _factory.getValidParams(mat_type);
    params.applyParameters(parameters(), {"scalar_global_strain"});
    params.set<bool>("use_displaced_mesh") = false;
    params.set<std::vector<VariableName>>("scalar_global_strain") = {
        getParam<VariableName>("scalar_global_strain")};
    params.set<UserObjectName>("global_strain_uo") = uo_name;

    _problem->addMaterial(mat_type, _name + "_global_strain", params);
  }

  //
  // Add auxiliary displacement variables
  //
  else if (_current_task == "add_aux_variable")
  {
    auto params = _factory.getValidParams("MooseVariable");
    // determine necessary order
    const bool second = _problem->mesh().hasSecondOrderElements();

    params.set<MooseEnum>("order") = second ? "SECOND" : "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    for (unsigned int i = 0; i < _aux_disp.size(); ++i)
    {
      std::string aux_var_name = _aux_disp[i];

      _problem->addAuxVariable("MooseVariable", aux_var_name, params);
    }

    for (unsigned int i = 0; i < _global_disp.size(); ++i)
    {
      std::string aux_var_name = _global_disp[i];

      _problem->addAuxVariable("MooseVariable", aux_var_name, params);
    }
  }

  //
  // Add aux kernels for computing global displacements
  //
  else if (_current_task == "add_aux_kernel")
  {
    for (unsigned int i = 0; i < _aux_disp.size(); ++i)
    {
      std::string aux_var_name = _aux_disp[i];

      std::string aux_type = "GlobalDisplacementAux";
      InputParameters params = _factory.getValidParams(aux_type);
      params.applyParameters(parameters(), {"scalar_global_strain"});
      params.set<AuxVariableName>("variable") = aux_var_name;
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<bool>("use_displaced_mesh") = false;
      params.set<bool>("output_global_displacement") = false;
      params.set<std::vector<VariableName>>("scalar_global_strain") = {
          getParam<VariableName>("scalar_global_strain")};
      params.set<UserObjectName>("global_strain_uo") = uo_name;
      params.set<unsigned int>("component") = i;

      _problem->addAuxKernel(aux_type, aux_var_name + '_' + name(), params);
    }

    for (unsigned int i = 0; i < _global_disp.size(); ++i)
    {
      std::string aux_var_name = _global_disp[i];

      std::string aux_type = "GlobalDisplacementAux";
      InputParameters params = _factory.getValidParams(aux_type);
      params.applyParameters(parameters(), {"scalar_global_strain"});
      params.set<AuxVariableName>("variable") = aux_var_name;
      params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
      params.set<bool>("use_displaced_mesh") = false;
      params.set<bool>("output_global_displacement") = true;
      params.set<std::vector<VariableName>>("scalar_global_strain") = {
          getParam<VariableName>("scalar_global_strain")};
      params.set<UserObjectName>("global_strain_uo") = uo_name;
      params.set<unsigned int>("component") = i;

      _problem->addAuxKernel(aux_type, aux_var_name + '_' + name(), params);
    }
  }
}
