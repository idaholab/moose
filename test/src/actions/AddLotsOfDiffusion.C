//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLotsOfDiffusion.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "DirichletBC.h"
#include "AddVariableAction.h"

#include <sstream>
#include <stdexcept>

#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_variable");
registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_kernel");
registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_bc");
registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_material");

InputParameters
AddLotsOfDiffusion::validParams()
{
  InputParameters params = Action::validParams();

  MooseEnum order(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH", "FIRST", true);
  params.addParam<MooseEnum>("order",
                             order,
                             "Order of the FE shape function to use for this variable (additional "
                             "orders not listed here are allowed, depending on the family).");

  MooseEnum family("LAGRANGE MONOMIAL HERMITE SCALAR HIERARCHIC CLOUGH XYZ SZABAB BERNSTEIN "
                   "L2_LAGRANGE L2_HIERARCHIC NEDELEC_ONE LAGRANGE_VEC",
                   "LAGRANGE");
  params.addParam<MooseEnum>(
      "family", family, "Specifies the family of FE shape functions to use for this variable.");
  params.addParam<bool>("array", false, "Whether or not to use array variables");
  params.addRequiredParam<unsigned int>("number", "The number of variables to add");
  params.addParam<unsigned int>("n_components", 2, "The number of components of array variables");
  params.addParam<bool>("add_reaction", false, "True to add reaction kernels");

  params.addRequiredParam<RealEigenVector>("diffusion_coefficients",
                                           "Diffusion coefficient to be used by all variables");
  params.declareControllable("diffusion_coefficients");
  return params;
}

AddLotsOfDiffusion::AddLotsOfDiffusion(const InputParameters & params) : Action(params) {}

void
AddLotsOfDiffusion::act()
{
  unsigned int number = getParam<unsigned int>("number");
  unsigned int ncomp = getParam<unsigned int>("n_components");

  const auto array = getParam<bool>("array");

  if (_current_task == "add_variable")
  {
    auto fe_type = AddVariableAction::feType(_pars);
    auto type = AddVariableAction::variableType(fe_type, false, array);
    auto var_params = _factory.getValidParams(type);

    var_params.set<MooseEnum>("family") = getParam<MooseEnum>("family");
    var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    if (array)
      var_params.set<unsigned int>("components") = ncomp;
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);
      _problem->addVariable(type, var_name, var_params);
    }
  }
  else if (_current_task == "add_kernel")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);
      {
        const auto kernel_type = array ? "ArrayDiffusion" : "Diffusion";

        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        if (array)
          params.set<MaterialPropertyName>("diffusion_coefficient") = "dc";
        _problem->addKernel(kernel_type, var_name, params);
      }

      if (getParam<bool>("add_reaction"))
      {
        const auto kernel_type = array ? "ArrayReaction" : "Reaction";
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        if (array)
          params.set<MaterialPropertyName>("reaction_coefficient") = "rc";
        _problem->addKernel(kernel_type, var_name + "_reaction", params);
      }
    }
  }
  else if (_current_task == "add_bc")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);
      const auto bc_type = array ? "ArrayDirichletBC" : "DirichletBC";

      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<BoundaryName>>("boundary").push_back("left");
      if (array)
        params.set<RealEigenVector>("values") = RealEigenVector::Constant(ncomp, 0);
      else
        params.set<Real>("value") = 0;

      _problem->addBoundaryCondition(bc_type, var_name + "_left", params);

      params.set<std::vector<BoundaryName>>("boundary")[0] = "right";
      if (array)
        params.set<RealEigenVector>("values") = RealEigenVector::Constant(ncomp, 1);
      else
        params.set<Real>("value") = 1;

      _problem->addBoundaryCondition(bc_type, var_name + "_right", params);
    }
  }
  else if (_current_task == "add_material" && array)
  {
    {
      auto params = _factory.getValidParams("GenericConstantArray");
      params.set<std::string>("prop_name") = "dc";
      params.set<RealEigenVector>("prop_value") =
          getParam<RealEigenVector>("diffusion_coefficients");
      _problem->addMaterial("GenericConstantArray", "dc", params);

      // pass the control to the material by connecting them
      connectControllableParams(
          "diffusion_coefficients", "GenericConstantArray", "dc", "prop_value");
    }

    if (getParam<bool>("add_reaction"))
    {
      auto params = _factory.getValidParams("GenericConstantArray");
      params.set<std::string>("prop_name") = "rc";
      params.set<RealEigenVector>("prop_value") = RealEigenVector::Constant(ncomp, 1);
      _problem->addMaterial("GenericConstantArray", "rc", params);
    }
  }
}
