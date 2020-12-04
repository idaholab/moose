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

InputParameters
GeneralizedPlaneStrainActionPD::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Class for setting up the Kernel, ScalarKernel, and UserObject for "
                             "peridynamic generalized plane strain model");

  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Nonlinear variable name for the displacements");
  params.addRequiredParam<VariableName>("scalar_out_of_plane_strain",
                                        "Scalar variable for strain in the out-of-plane direction");
  params.addParam<VariableName>("temperature", "Nonlinear variable for the temperature");
  MooseEnum formulation_option("ORDINARY_STATE NONORDINARY_STATE", "NONORDINARY_STATE");
  params.addParam<MooseEnum>("formulation", formulation_option, "Peridynamic formulation options");
  MooseEnum strain_type("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strain_type, "Strain formulation");
  params.addParam<VariableName>("out_of_plane_stress_variable",
                                "Name of out-of-plane stress auxiliary variable");
  params.addParam<FunctionName>(
      "out_of_plane_pressure",
      "0",
      "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed out-of-plane pressure");
  params.addParam<bool>("full_jacobian",
                        false,
                        "Parameter to set whether to use the nonlocal full Jacobian formulation "
                        "for the scalar components");
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
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _formulation(getParam<MooseEnum>("formulation")),
    _scalar_out_of_plane_strain(getParam<VariableName>("scalar_out_of_plane_strain"))
{
  // Generalized plane strain only applies to two dimensional modeling and simulation
  if (_ndisp != 2)
    mooseError("GeneralizedPlaneStrainPD only works for two dimensional case!");

  // Consistency check
  if (_formulation == "NONORDINARY_STATE" && isParamValid("out_of_plane_stress_variable"))
    mooseWarning("Variable out_of_plane_stress_variable will not be used in NONORDINARY_STATE "
                 "formulation option!");
  if (_formulation == "ORDINARY_STATE" && !isParamValid("out_of_plane_stress_variable"))
    mooseError("Variable out_of_plane_stress_variable must be provided for ORDINARY_STATE "
               "formulation option!");
}

void
GeneralizedPlaneStrainActionPD::act()
{
  if (_current_task == "add_kernel")
  {
    std::string k_type;
    if (_formulation == "ORDINARY_STATE")
      k_type = "GeneralizedPlaneStrainOffDiagOSPD"; // Based on the ordinary state-based model
    else if (_formulation == "NONORDINARY_STATE")
      k_type = "GeneralizedPlaneStrainOffDiagNOSPD"; // Based on Form I of horizon-stabilized
                                                     // correspondence model
    else
      paramError("formulation", "Unsupported peridynamic formulation");

    InputParameters params = _factory.getValidParams(k_type);

    params.applyParameters(parameters(),
                           {"displacements", "temperature", "scalar_out_of_plane_strain"});

    params.set<std::vector<VariableName>>("displacements") = _displacements;
    params.set<std::vector<VariableName>>("scalar_out_of_plane_strain") = {
        _scalar_out_of_plane_strain};

    // Coupling between scalar out-of-plane strain and in-plane displacements
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      std::string k_name = name() + "_GeneralizedPlaneStrainPDOffDiag_disp_" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      _problem->addKernel(k_type, k_name, params);
    }

    // Coupling between scalar out-of-plane strain and temperature (only when temperature is a
    // nonlinear variable)
    if (isParamValid("temperature"))
    {
      VariableName temp = getParam<VariableName>("temperature");
      if (_problem->getNonlinearSystemBase().hasVariable(temp))
      {
        params.set<std::vector<VariableName>>("temperature") = {temp};

        std::string k_name = name() + "_GeneralizedPlaneStrainPDOffDiag_temp";
        params.set<NonlinearVariableName>("variable") = temp;

        if (_formulation == "NONORDINARY_STATE")
          params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
              getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");

        _problem->addKernel(k_type, k_name, params);
      }
    }
  }
  else if (_current_task == "add_user_object")
  {
    std::string uo_type;
    if (_formulation == "ORDINARY_STATE")
      uo_type = "GeneralizedPlaneStrainUserObjectOSPD";
    else if (_formulation == "NONORDINARY_STATE")
      uo_type = "GeneralizedPlaneStrainUserObjectNOSPD";
    else
      paramError("formulation", "Unsupported peridynamic formulation!");

    InputParameters params = _factory.getValidParams(uo_type);

    std::string uo_name = name() + "_GeneralizedPlaneStrainPDUserObject";

    params.applyParameters(parameters(), {"out_of_plane_stress_variable"});

    if (_formulation == "ORDINARY_STATE")
      params.set<std::vector<VariableName>>("out_of_plane_stress_variable") = {
          getParam<VariableName>("out_of_plane_stress_variable")};

    _problem->addUserObject(uo_type, uo_name, params);
  }
  else if (_current_task == "add_scalar_kernel")
  {
    std::string sk_type("GeneralizedPlaneStrainPD");
    InputParameters params = _factory.getValidParams(sk_type);

    std::string sk_name = name() + "_GeneralizedPlaneStrainPD";

    params.set<NonlinearVariableName>("variable") = _scalar_out_of_plane_strain;

    // set the UserObjectName using added UserObject
    params.set<UserObjectName>("generalized_plane_strain_uo") =
        name() + "_GeneralizedPlaneStrainPDUserObject";

    _problem->addScalarKernel(sk_type, sk_name, params);
  }
  else
    mooseError("Task error in GeneralizedPlaneStrainActionPD!");
}
