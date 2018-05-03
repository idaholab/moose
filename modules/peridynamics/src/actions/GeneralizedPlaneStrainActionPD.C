//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainActionPD.h"
#include "NonlinearSystemBase.h"
#include "Factory.h"
#include "FEProblemBase.h"
#include "MooseObjectAction.h"

registerMooseAction("PeridynamicsApp", GeneralizedPlaneStrainActionPD, "add_kernel");
registerMooseAction("PeridynamicsApp", GeneralizedPlaneStrainActionPD, "add_user_object");
registerMooseAction("PeridynamicsApp", GeneralizedPlaneStrainActionPD, "add_scalar_kernel");

template <>
InputParameters
validParams<GeneralizedPlaneStrainActionPD>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Class for setting up the Kernel, ScalarKernel, and UserObject for "
                             "peridynamic generalized plane strain model");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable name for the displacements");
  params.addRequiredParam<VariableName>("scalar_out_of_plane_strain",
                                        "Scalar variable for strain in the out-of-plane direction");
  params.addParam<NonlinearVariableName>("temperature", "Nonlinear variable for the temperature");
  MooseEnum formulation_option("OrdinaryState NonOrdinaryState", "NonOrdinaryState");
  params.addParam<MooseEnum>("formulation",
                             formulation_option,
                             "Available peridynamic formulation options: " +
                                 formulation_option.getRawNames());
  params.addParam<AuxVariableName>("out_of_plane_stress_variable",
                                   "Name of out-of-plane stress auxiliary variable");
  params.addParam<FunctionName>(
      "out_of_plane_pressure",
      "0",
      "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed out-of-plane pressure");
  params.addParam<bool>("full_jacobian",
                        false,
                        "Parameter to set whether to use the nonlocal full jacobian formulation "
                        "for the scalar components or not");
  params.addParam<bool>(
      "use_displaced_mesh",
      false,
      "Parameter to set whether to use the displaced mesh for computation or not");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "List of ids of the blocks (subdomains) that the "
                                              "GeneralizedPlaneStrainActionPD will be applied "
                                              "to");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");

  return params;
}

GeneralizedPlaneStrainActionPD::GeneralizedPlaneStrainActionPD(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _formulation(getParam<MooseEnum>("formulation")),
    _scalar_out_of_plane_strain(getParam<VariableName>("scalar_out_of_plane_strain"))
{
  // Generalized plane strain only applies to two dimensional modeling and simulation
  if (_ndisp != 2)
    mooseError("GeneralizedPlaneStrainPD only works for two dimensional case!");

  // Consistency check
  if (_formulation == "NonOrdinaryState" && isParamValid("out_of_plane_stress_variable"))
    mooseWarning("Variable out_of_plane_stress_variable will not be used in NonOrdinaryState "
                 "formulation option!");
  if (_formulation == "OrdinaryState" && !isParamValid("out_of_plane_stress_variable"))
    mooseError("Variable out_of_plane_stress_variable must be provided for OrdinaryState "
               "formulation option!");
}

void
GeneralizedPlaneStrainActionPD::act()
{
  if (_current_task == "add_kernel")
  {
    std::string k_type;
    switch (_formulation)
    {
      case 0:
        k_type = "GeneralizedPlaneStrainOffDiagOSPD"; // Based on ordinary state-based model
        break;
      case 1:
        k_type = "GeneralizedPlaneStrainOffDiagNOSPD"; // Based on bond-associated non-rodinary
                                                       // state-based model
        break;
      default:
        mooseError("Unsupported PD formulation. Choose from: OrdinaryState or NonOrdinaryState");
    }

    InputParameters params = _factory.getValidParams(k_type);

    params.set<std::vector<NonlinearVariableName>>("displacements") = _displacements;
    params.set<std::vector<VariableName>>("scalar_out_of_plane_strain") = {
        _scalar_out_of_plane_strain};
    params.set<bool>("full_jacobian") = getParam<bool>("full_jacobian");

    if (isParamValid("block"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("block");

    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    // Coupling between scalar out-of-plane strain and in-plane displacements
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      std::string k_name = _name + "_GeneralizedPlaneStrainPDOffDiag_disp_" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      _problem->addKernel(k_type, k_name, params);
    }

    // Coupling between scalar out-of-plane strain and temperature (only when temperature is a
    // nonlinear variable)
    if (isParamValid("temperature"))
    {
      NonlinearVariableName temp = getParam<NonlinearVariableName>("temperature");
      if (_problem->getNonlinearSystemBase().hasVariable(temp))
      {
        params.set<NonlinearVariableName>("temperature") = temp;

        std::string k_name = _name + "_GeneralizedPlaneStrainPDOffDiag_temp";
        params.set<NonlinearVariableName>("variable") = temp;
        if (_formulation == "NonOrdinaryState")
        {
          params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
              getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");
        }

        _problem->addKernel(k_type, k_name, params);
      }
    }
  }
  else if (_current_task == "add_user_object")
  {
    std::string uo_type;
    switch (_formulation)
    {
      case 0:
        uo_type = "GeneralizedPlaneStrainUserObjectOSPD";
        break;
      case 1:
        uo_type = "GeneralizedPlaneStrainUserObjectNOSPD";
        break;
      default:
        mooseError("Unsupported PD formulation. Choose from: OrdinaryState or NonOrdinaryState");
    }

    InputParameters params = _factory.getValidParams(uo_type);

    std::string uo_name = _name + "_GeneralizedPlaneStrainPDUserObject";

    params.set<FunctionName>("out_of_plane_pressure") =
        getParam<FunctionName>("out_of_plane_pressure");
    params.set<Real>("factor") = getParam<Real>("factor");

    if (_formulation == "OrdinaryState")
      params.set<AuxVariableName>("out_of_plane_stress_variable") =
          getParam<AuxVariableName>("out_of_plane_stress_variable");

    if (isParamValid("block"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("block");

    params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;
    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    _problem->addUserObject(uo_type, uo_name, params);
  }
  else if (_current_task == "add_scalar_kernel")
  {
    std::string sk_type("GeneralizedPlaneStrainPD");
    InputParameters params = _factory.getValidParams(sk_type);

    std::string sk_name = _name + "_GeneralizedPlaneStrainPD";

    params.set<NonlinearVariableName>("variable") = _scalar_out_of_plane_strain;
    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    // set the UserObjectName from previously added UserObject
    params.set<UserObjectName>("generalized_plane_strain_uo") =
        _name + "_GeneralizedPlaneStrainPDUserObject";

    _problem->addScalarKernel(sk_type, sk_name, params);
  }
  else
    mooseError("Task error in GeneralizedPlaneStrainActionPD");
}
