//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrandPotentialKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", GrandPotentialKernelAction, "add_kernel");

InputParameters
GrandPotentialKernelAction::validParams()
{
  InputParameters parameters = Action::validParams();
  parameters.addClassDescription(
      "Automatically generate most or all of the kernels for the grand potential model");
  parameters.addRequiredParam<std::vector<NonlinearVariableName>>(
      "chemical_potentials", "List of chemical potential variables");
  parameters.addRequiredParam<std::vector<MaterialPropertyName>>(
      "susceptibilities", "List of susceptibilities that correspond to chemical_potentials");
  parameters.addRequiredParam<std::vector<MaterialPropertyName>>(
      "free_energies_gr",
      "List of free energies for each phase. Place in same order as switching_function_names.");
  parameters.addRequiredParam<std::vector<MaterialPropertyName>>(
      "free_energies_w",
      "List of functions for each phase. Length should be length of chemical_potentials * length "
      "of switching_function_names.");
  parameters.addRequiredParam<std::vector<MaterialPropertyName>>(
      "switching_function_names",
      "Switching function materials that provide switching function for free_energies_*.");
  parameters.addRequiredParam<std::vector<MaterialPropertyName>>(
      "mobilities", "Vector of mobilities that must match chemical_potentials");
  parameters.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  parameters.addRequiredParam<std::string>("var_name_base",
                                           "specifies the base name of the grain variables");
  parameters.addParam<std::vector<NonlinearVariableName>>(
      "additional_ops", "List of any additional order parameters which are not grains");
  parameters.addParam<std::vector<MaterialPropertyName>>("free_energies_op",
                                                         "List of free energies used by additional "
                                                         "order parameters. Places in same order "
                                                         "as switching_function_names.");
  parameters.addParam<MaterialPropertyName>("kappa_gr", "kappa", "The kappa used with the grains");
  parameters.addParam<MaterialPropertyName>(
      "kappa_op", "kappa", "The kappa used with additional_ops");
  parameters.addParam<MaterialPropertyName>(
      "gamma_gr", "gamma", "Name of the gamma used with grains");
  parameters.addParam<MaterialPropertyName>(
      "gamma_op", "gamma", "Name of the gamma used with additional order parameters");
  parameters.addParam<MaterialPropertyName>(
      "gamma_grxop",
      "gamma",
      "Name of the gamma used when grains interact with other order parameters");
  parameters.addParam<MaterialPropertyName>(
      "mobility_name_gr", "L", "Name of mobility to be used with grains");
  parameters.addParam<MaterialPropertyName>(
      "mobility_name_op", "L", "Name of mobility to be used with additional_ops");
  parameters.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  parameters.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  MultiMooseEnum anisotropy("true=1 false=0", "false");
  parameters.addParam<bool>(
      "mass_conservation", false, "Whether strict mass conservation formulation is used or not");
  parameters.addRequiredParam<MultiMooseEnum>(
      "anisotropic", anisotropy, "Whether or not each mobility is anisotropic");
  parameters.addParam<std::vector<NonlinearVariableName>>(
      "concentrations", "List of concentration variables for strict mass conservation");
  parameters.addParam<std::vector<MaterialPropertyName>>(
      "hj_c_min",
      "List of body forces coefficients for strict mass conservation formulation that indicates "
      "the minima in concentration free energy."
      "Place in same order as switching_function_names.");
  parameters.addParam<std::vector<MaterialPropertyName>>(
      "hj_over_kVa",
      "List of MatReaction coefficients for strict mass conservation formulation that relates "
      "chemical potential with switching functionj between phases"
      "Place in same order as switching_function_names.");
  return parameters;
}

GrandPotentialKernelAction::GrandPotentialKernelAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
GrandPotentialKernelAction::act()
{
  // Get Variables from parameters
  const auto w_names = getParam<std::vector<NonlinearVariableName>>("chemical_potentials");
  const auto chis = getParam<std::vector<MaterialPropertyName>>("susceptibilities");
  const auto Fj_gr = getParam<std::vector<MaterialPropertyName>>("free_energies_gr");
  const auto Fj_w = getParam<std::vector<MaterialPropertyName>>("free_energies_w");
  const auto hj = getParam<std::vector<MaterialPropertyName>>("switching_function_names");
  const auto M = getParam<std::vector<MaterialPropertyName>>("mobilities");
  auto n_grs = getParam<unsigned int>("op_num");
  const auto var_name_base = getParam<std::string>("var_name_base");
  const auto Fj_op = getParam<std::vector<MaterialPropertyName>>("free_energies_op");
  const auto kappa_gr = getParam<MaterialPropertyName>("kappa_gr");
  const auto kappa_op = getParam<MaterialPropertyName>("kappa_op");
  const auto gamma_gr = getParam<MaterialPropertyName>("gamma_gr");
  const auto gamma_op = getParam<MaterialPropertyName>("gamma_op");
  const auto gamma_xx = getParam<MaterialPropertyName>("gamma_grxop");
  const auto gr_mob = getParam<MaterialPropertyName>("mobility_name_gr");
  const auto op_mob = getParam<MaterialPropertyName>("mobility_name_op");
  auto implicity = getParam<bool>("implicit");
  auto displaced_mesh = getParam<bool>("use_displaced_mesh");
  auto aniso = getParam<MultiMooseEnum>("anisotropic");
  const auto hj_over_kVa = getParam<std::vector<MaterialPropertyName>>("hj_over_kVa");
  const auto hj_c_min = getParam<std::vector<MaterialPropertyName>>("hj_c_min");
  auto mass_conservation = getParam<bool>("mass_conservation");

  // Size definitions and checks
  unsigned int n_w = w_names.size();
  unsigned int n_hj = hj.size();
  std::vector<NonlinearVariableName> etas;
  std::vector<NonlinearVariableName> c_names;
  unsigned int n_etas = 0;
  std::string kernel_name;
  if (isParamValid("additional_ops"))
  {
    etas = getParam<std::vector<NonlinearVariableName>>("additional_ops");
    n_etas = etas.size();
  }

  if (chis.size() != n_w)
    mooseError("susceptibilities and chemical_potentials should be vectors of the same length.");
  if (Fj_w.size() != n_w * n_hj)
    mooseError("free_energies_w should be length of chemcial_potentials * length of "
               "switching_function_names");
  if (M.size() != n_w)
    mooseError("M and chemical_potentials should be vectors of the same length.");
  if (aniso.size() != n_w)
    paramError("anisotropic", "Provide as many values as entries in 'chemical_potentials'.");

  // Define additional vectors
  std::vector<std::string> grs; // vector of all grain variable names
  grs.resize(n_grs);
  for (unsigned int i = 0; i < n_grs; ++i)
    grs[i] = var_name_base + Moose::stringify(i);

  std::vector<NonlinearVariableName> all_etas; // vector of all grain variables and order parameters
  all_etas.reserve(n_etas + n_grs);
  all_etas.insert(all_etas.end(), etas.begin(), etas.end());
  all_etas.insert(all_etas.end(), grs.begin(), grs.end());

  std::vector<std::string> all_vars; // vector of all variables
  all_vars.reserve(n_etas + n_grs + n_w);
  all_vars.insert(all_vars.end(), all_etas.begin(), all_etas.end());
  all_vars.insert(all_vars.end(), w_names.begin(), w_names.end());

  std::vector<MaterialPropertyName> fj_temp;
  fj_temp.resize(n_hj);
  std::vector<VariableName> notarealvector;
  notarealvector.resize(1);
  std::vector<VariableName> v0;
  v0.resize(n_etas + n_grs + n_w);
  for (unsigned int i = 0; i < n_etas + n_grs + n_w; ++i)
    v0[i] = all_vars[i];
  std::vector<VariableName> v1;
  v1.resize(n_etas + n_grs);
  for (unsigned int i = 0; i < n_etas + n_grs; ++i)
    v1[i] = all_etas[i];
  std::vector<VariableName> v2;
  v2.resize(n_etas + n_grs - 1);

  // Grains and order parameters
  NonlinearVariableName var_name;
  MaterialPropertyName kappa;
  MaterialPropertyName mob_name;
  std::vector<MaterialPropertyName> Fj_names;

  for (unsigned int i = 0; i < n_etas + n_grs; ++i)
  {
    var_name = all_etas[i];
    // Distinguish between grains and the additional order parameters
    if (i < n_etas) // First part of list is grain variables
    {
      kappa = kappa_op;
      mob_name = op_mob;
      Fj_names.resize(Fj_op.size());
      Fj_names = Fj_op;
    }
    else // Second part of list is additional order parameters
    {
      kappa = kappa_gr;
      mob_name = gr_mob;
      Fj_names.resize(Fj_gr.size());
      Fj_names = Fj_gr;
    }

    // Remove var_name from coupled variables
    std::vector<MaterialPropertyName> gam;
    gam.resize(n_etas + n_grs - 1);
    unsigned int op = 0;
    for (unsigned int j = 0; j < n_etas + n_grs; ++j)
    {
      if (i != j)
      {
        v2[op] = all_etas[j];
        if (j < n_etas)
          gam[op] = gamma_op;
        else
          gam[op] = gamma_gr;
        if (i < n_etas && j < n_etas)
          gam[op] = gamma_op;
        else if (i >= n_etas && j >= n_etas)
          gam[op] = gamma_gr;
        else
          gam[op] = gamma_xx;
        ++op;
      }
    }

    // TimeDerivative Kernel
    InputParameters params = _factory.getValidParams("TimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = implicity;
    params.set<bool>("use_displaced_mesh") = displaced_mesh;
    kernel_name = "DT_" + var_name;
    _problem->addKernel("TimeDerivative", kernel_name, params);

    // ACInterface Kernel
    params = _factory.getValidParams("ACInterface");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = implicity;
    params.set<bool>("use_displaced_mesh") = displaced_mesh;
    params.set<MaterialPropertyName>("kappa_name") = kappa;
    params.set<MaterialPropertyName>("mob_name") = mob_name;
    params.set<std::vector<VariableName>>("coupled_variables") = v2;
    kernel_name = "ACInt_" + var_name;
    _problem->addKernel("ACInterface", kernel_name, params);

    // ACSwitching Kernel
    params = _factory.getValidParams("ACSwitching");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = implicity;
    params.set<bool>("use_displaced_mesh") = displaced_mesh;
    params.set<std::vector<MaterialPropertyName>>("Fj_names") = Fj_names;
    params.set<std::vector<MaterialPropertyName>>("hj_names") = hj;
    params.set<MaterialPropertyName>("mob_name") = mob_name;
    params.set<std::vector<VariableName>>("coupled_variables") = v0;
    kernel_name = "ACSwitch_" + var_name;
    _problem->addKernel("ACSwitching", kernel_name, params);

    // ACGrGrMulti Kernel
    params = _factory.getValidParams("ACGrGrMulti");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = implicity;
    params.set<bool>("use_displaced_mesh") = displaced_mesh;
    params.set<MaterialPropertyName>("mob_name") = mob_name;
    params.set<std::vector<VariableName>>("v") = v2;
    params.set<std::vector<MaterialPropertyName>>("gamma_names") = gam;
    kernel_name = "AcGrGr_" + var_name;
    _problem->addKernel("ACGrGrMulti", kernel_name, params);
  } // for (unsigned int i = 0; i < n_etas + n_grs; ++i)

  if (mass_conservation) // mass conservation kernels with conc and chempot coupling
  {
    c_names = getParam<std::vector<NonlinearVariableName>>("concentrations");
    std::vector<VariableName> v3;
    v3.resize(n_w);
    for (unsigned int i = 0; i < n_w; ++i)
      v3[i] = w_names[i];

    std::vector<VariableName> v4;
    unsigned int n_c = c_names.size();
    v4.resize(n_c);
    for (unsigned int i = 0; i < n_c; ++i)
      v4[i] = c_names[i];

    for (unsigned int i = 0; i < n_c; ++i)
    {
      // TimeDerivative concentration Kernel
      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = c_names[i];
      params.set<bool>("implicit") = implicity;
      params.set<bool>("use_displaced_mesh") = displaced_mesh;
      kernel_name = "DT_" + c_names[i];
      _problem->addKernel("TimeDerivative", kernel_name, params);

      // MatDiffusion concentration (coupled with chempot)
      params = _factory.getValidParams("MatDiffusion");
      params.set<NonlinearVariableName>("variable") = c_names[i];
      params.set<std::vector<VariableName>>("v") = v3;
      params.set<bool>("implicit") = implicity;
      params.set<bool>("use_displaced_mesh") = displaced_mesh;
      params.set<MaterialPropertyName>("diffusivity") = M[i];
      kernel_name = "MatDif_" + w_names[i];
      if (aniso.get(i))
      {
        params.set<std::vector<VariableName>>("args") = v1;
        _problem->addKernel("MatAnisoDiffusion", kernel_name, params);
      }
      else
      {
        params.set<std::vector<VariableName>>("args") = v0;
        _problem->addKernel("MatDiffusion", kernel_name, params);
      }
    }

    // Chemical Potentials
    for (unsigned int i = 0; i < n_w; ++i)
    {
      // coupling of c and w
      InputParameters params = _factory.getValidParams("MatReaction");
      params.set<NonlinearVariableName>("variable") = w_names[i];
      params.set<std::vector<VariableName>>("v") = v4;
      params.set<MaterialPropertyName>("mob_name") = "-1";
      kernel_name = "MR_c" + w_names[i];
      _problem->addKernel("MatReaction", kernel_name, params);

      // contribution between chempot and each grains to concentration
      // Summations of MatReaction and MaskedBodyForce

      for (unsigned int j = 0; j < n_hj; ++j)
      {
        // MatReaction
        params = _factory.getValidParams("MatReaction");
        params.set<NonlinearVariableName>("variable") = w_names[i];
        params.set<std::vector<VariableName>>("args") = v1;
        params.set<MaterialPropertyName>("mob_name") = hj_over_kVa[j];
        params.set<bool>("implicit") = implicity;
        params.set<bool>("use_displaced_mesh") = displaced_mesh;
        kernel_name = "MR_" + w_names[i] + "_" + all_etas[j];
        _problem->addKernel("MatReaction", kernel_name, params);

        // MaskedBodyForce
        InputParameters params = _factory.getValidParams("MaskedBodyForce");
        params.set<NonlinearVariableName>("variable") = w_names[i];
        params.set<std::vector<VariableName>>("coupled_variables") = v1;
        params.set<MaterialPropertyName>("mask") = hj_c_min[j];
        params.set<bool>("implicit") = implicity;
        params.set<bool>("use_displaced_mesh") = displaced_mesh;
        kernel_name = "MBD_" + w_names[i] + "_" + all_etas[j];
        _problem->addKernel("MaskedBodyForce", kernel_name, params);
      }
    }
  }
  else
  {
    // Chemical Potentials
    for (unsigned int i = 0; i < n_w; ++i)
    {
      // SusceptibilityTimeDerivative
      InputParameters params = _factory.getValidParams("SusceptibilityTimeDerivative");
      params.set<NonlinearVariableName>("variable") = w_names[i];
      params.set<MaterialPropertyName>("f_name") = chis[i];
      params.set<std::vector<VariableName>>("coupled_variables") = v0;
      params.set<bool>("implicit") = implicity;
      params.set<bool>("use_displaced_mesh") = displaced_mesh;
      kernel_name = "ChiDt_" + w_names[i];
      _problem->addKernel("SusceptibilityTimeDerivative", kernel_name, params);

      // MatDiffusion
      params = _factory.getValidParams("MatDiffusion");
      params.set<NonlinearVariableName>("variable") = w_names[i];
      params.set<bool>("implicit") = implicity;
      params.set<bool>("use_displaced_mesh") = displaced_mesh;
      params.set<MaterialPropertyName>("diffusivity") = M[i];
      kernel_name = "MatDif_" + w_names[i];
      if (aniso.get(i))
        _problem->addKernel("MatAnisoDiffusion", kernel_name, params);
      else
      {
        params.set<std::vector<VariableName>>("args") = v0;
        _problem->addKernel("MatDiffusion", kernel_name, params);
      }

      // CoupledSwitchingTimeDerivative
      for (unsigned int j = 0; j < n_hj; ++j)
        fj_temp[j] = Fj_w[i * n_hj + j];
      for (unsigned int j = 0; j < n_etas + n_grs; ++j)
      {
        notarealvector[0] = all_etas[j];
        params = _factory.getValidParams("CoupledSwitchingTimeDerivative");
        params.set<NonlinearVariableName>("variable") = w_names[i];
        params.set<std::vector<VariableName>>("v") = notarealvector;
        params.set<std::vector<VariableName>>("coupled_variables") = v0;
        params.set<std::vector<MaterialPropertyName>>("Fj_names") = fj_temp;
        params.set<std::vector<MaterialPropertyName>>("hj_names") = hj;
        params.set<bool>("implicit") = implicity;
        params.set<bool>("use_displaced_mesh") = displaced_mesh;
        kernel_name = "Coupled_" + w_names[i] + "_" + all_etas[j];
        _problem->addKernel("CoupledSwitchingTimeDerivative", kernel_name, params);
      }
    }
  }
} // GrandPotentialKernelAction::act()
