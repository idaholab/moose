//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowActionBase.h"

#include "FEProblem.h"
#include "MooseMesh.h"
#include "libmesh/string_to_enum.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PorousFlowActionBase>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>(
      "dictator_name",
      "dictator",
      "The name of the dictator user object that is created by this Action");
  params.addClassDescription("Adds the PorousFlowDictator UserObject.  This class also contains "
                             "many utility functions for adding other pieces of an input file, "
                             "which may be used by derived classes.");
  params.addParam<RealVectorValue>("gravity",
                                   RealVectorValue(0.0, 0.0, -10.0),
                                   "Gravitational acceleration vector downwards (m/s^2)");
  params.addCoupledVar("temperature",
                       293.0,
                       "For isothermal simulations, this is the temperature "
                       "at which fluid properties (and stress-free strains) "
                       "are evaluated at.  Otherwise, this is the name of "
                       "the temperature variable.  Units = Kelvin");
  params.addCoupledVar("mass_fraction_vars",
                       "List of variables that represent the mass fractions.  Format is 'f_ph0^c0 "
                       "f_ph0^c1 f_ph0^c2 ... f_ph0^c(N-1) f_ph1^c0 f_ph1^c1 fph1^c2 ... "
                       "fph1^c(N-1) ... fphP^c0 f_phP^c1 fphP^c2 ... fphP^c(N-1)' where "
                       "N=num_components and P=num_phases, and it is assumed that "
                       "f_ph^cN=1-sum(f_ph^c,{c,0,N-1}) so that f_ph^cN need not be given.  If no "
                       "variables are provided then num_phases=1=num_components.");
  params.addParam<unsigned int>("number_aqueous_equilibrium",
                                0,
                                "The number of secondary species in the aqueous-equilibrium "
                                "reaction system.  (Leave as zero if the simulation does not "
                                "involve chemistry)");
  params.addParam<unsigned int>("number_aqueous_kinetic",
                                0,
                                "The number of secondary species in the aqueous-kinetic reaction "
                                "system involved in precipitation and dissolution.  (Leave as zero "
                                "if the simulation does not involve chemistry)");
  params.addParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The name of the displacement variables (relevant only for "
      "mechanically-coupled simulations)");
  params.addParam<std::string>("thermal_eigenstrain_name",
                               "thermal_eigenstrain",
                               "The eigenstrain_name used in the "
                               "ComputeThermalExpansionEigenstrain.  Only needed for "
                               "thermally-coupled simulations with thermal expansion.");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Use displaced mesh computations in mechanical kernels");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "VanLeer");
  params.addParam<MooseEnum>(
      "flux_limiter_type",
      flux_limiter_type,
      "Type of flux limiter to use if stabilization=KT.  'None' means that no antidiffusion "
      "will be added in the Kuzmin-Turek scheme");
  MooseEnum stabilization("Full KT", "Full");
  params.addParam<MooseEnum>("stabilization",
                             stabilization,
                             "Numerical stabilization used.  'Full' means full upwinding.  'KT' "
                             "means FEM-TVD stabilization of Kuzmin-Turek");
  return params;
}

PorousFlowActionBase::PorousFlowActionBase(const InputParameters & params)
  : Action(params),
    PorousFlowDependencies(),
    _objects_to_add(),
    _dictator_name(getParam<std::string>("dictator_name")),
    _num_aqueous_equilibrium(getParam<unsigned int>("number_aqueous_equilibrium")),
    _num_aqueous_kinetic(getParam<unsigned int>("number_aqueous_kinetic")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _mass_fraction_vars(getParam<std::vector<VariableName>>("mass_fraction_vars")),
    _num_mass_fraction_vars(_mass_fraction_vars.size()),
    _temperature_var(getParam<std::vector<VariableName>>("temperature")),
    _displacements(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _coupled_displacements(_ndisp),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type")),
    _stabilization(getParam<MooseEnum>("stabilization").getEnum<StabilizationEnum>())
{
  // convert vector of NonlinearVariableName to vector of VariableName
  for (unsigned int i = 0; i < _ndisp; ++i)
    _coupled_displacements[i] = _displacements[i];
}

void
PorousFlowActionBase::act()
{
  // Check if the simulation is transient (note: can't do this in the ctor)
  _transient = _problem->isTransient();

  // Make sure that all mesh subdomains have the same coordinate system
  const auto & all_subdomains = _problem->mesh().meshSubdomains();
  if (all_subdomains.empty())
    mooseError("No subdomains found");
  _coord_system = _problem->getCoordSystem(*all_subdomains.begin());
  for (const auto & subdomain : all_subdomains)
    if (_problem->getCoordSystem(subdomain) != _coord_system)
      mooseError(
          "The PorousFlow Actions require all subdomains to have the same coordinate system.");

  // Note: this must be called before addMaterials!
  addMaterialDependencies();

  if (_current_task == "add_user_object")
    addUserObjects();

  if (_current_task == "add_aux_variable" || _current_task == "add_aux_kernel")
    addAuxObjects();

  if (_current_task == "add_kernel")
    addKernels();

  if (_current_task == "add_material")
    addMaterials();
}

void
PorousFlowActionBase::addMaterialDependencies()
{
  // Check to see if there are any other PorousFlow objects like BCs that
  // may require specific versions of materials
}

void
PorousFlowActionBase::addUserObjects()
{
  addDictator();
}

void
PorousFlowActionBase::addAuxObjects()
{
}

void
PorousFlowActionBase::addKernels()
{
}

void
PorousFlowActionBase::addMaterials()
{
}

void
PorousFlowActionBase::addSaturationAux(unsigned phase)
{
  std::string phase_str = Moose::stringify(phase);

  if (_current_task == "add_aux_variable")
    _problem->addAuxVariable("saturation" + phase_str,
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));

  if (_current_task == "add_aux_kernel")
  {
    std::string aux_kernel_type = "MaterialStdVectorAux";
    InputParameters params = _factory.getValidParams(aux_kernel_type);

    std::string aux_kernel_name = "PorousFlowActionBase_SaturationAux" + phase_str;
    params.set<MaterialPropertyName>("property") = "PorousFlow_saturation_qp";
    params.set<unsigned>("index") = phase;
    params.set<AuxVariableName>("variable") = "saturation" + phase_str;
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);
  }
}

void
PorousFlowActionBase::addDarcyAux(const RealVectorValue & gravity)
{
  if (_current_task == "add_aux_variable")
  {
    _problem->addAuxVariable("darcy_vel_x",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("darcy_vel_y",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("darcy_vel_z",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
  }

  if (_current_task == "add_aux_kernel")
  {
    std::string aux_kernel_type = "PorousFlowDarcyVelocityComponent";
    InputParameters params = _factory.getValidParams(aux_kernel_type);

    params.set<RealVectorValue>("gravity") = gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

    std::string aux_kernel_name = "PorousFlowActionBase_Darcy_x_Aux";
    params.set<MooseEnum>("component") = "x";
    params.set<AuxVariableName>("variable") = "darcy_vel_x";
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowActionBase_Darcy_y_Aux";
    params.set<MooseEnum>("component") = "y";
    params.set<AuxVariableName>("variable") = "darcy_vel_y";
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowActionBase_Darcy_z_Aux";
    params.set<MooseEnum>("component") = "z";
    params.set<AuxVariableName>("variable") = "darcy_vel_z";
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);
  }
}

void
PorousFlowActionBase::addStressAux()
{
  if (_current_task == "add_aux_variable")
  {
    _problem->addAuxVariable("stress_xx",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_xy",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_xz",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_yx",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_yy",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_yz",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_zx",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_zy",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("stress_zz",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));
  }

  if (_current_task == "add_aux_kernel")
  {
    std::string aux_kernel_type = "RankTwoAux";
    InputParameters params = _factory.getValidParams(aux_kernel_type);

    params.set<MaterialPropertyName>("rank_two_tensor") = "stress";
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

    std::string aux_kernel_name = "PorousFlowAction_stress_xx";
    params.set<AuxVariableName>("variable") = "stress_xx";
    params.set<unsigned>("index_i") = 0;
    params.set<unsigned>("index_j") = 0;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_xy";
    params.set<AuxVariableName>("variable") = "stress_xy";
    params.set<unsigned>("index_i") = 0;
    params.set<unsigned>("index_j") = 1;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_xz";
    params.set<AuxVariableName>("variable") = "stress_xz";
    params.set<unsigned>("index_i") = 0;
    params.set<unsigned>("index_j") = 2;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_yx";
    params.set<AuxVariableName>("variable") = "stress_yx";
    params.set<unsigned>("index_i") = 1;
    params.set<unsigned>("index_j") = 0;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_yy";
    params.set<AuxVariableName>("variable") = "stress_yy";
    params.set<unsigned>("index_i") = 1;
    params.set<unsigned>("index_j") = 1;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_yz";
    params.set<AuxVariableName>("variable") = "stress_yz";
    params.set<unsigned>("index_i") = 1;
    params.set<unsigned>("index_j") = 2;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_zx";
    params.set<AuxVariableName>("variable") = "stress_zx";
    params.set<unsigned>("index_i") = 2;
    params.set<unsigned>("index_j") = 0;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_zy";
    params.set<AuxVariableName>("variable") = "stress_zy";
    params.set<unsigned>("index_i") = 2;
    params.set<unsigned>("index_j") = 1;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);

    aux_kernel_name = "PorousFlowAction_stress_zz";
    params.set<AuxVariableName>("variable") = "stress_zz";
    params.set<unsigned>("index_i") = 2;
    params.set<unsigned>("index_j") = 2;
    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);
  }
}

void
PorousFlowActionBase::addTemperatureMaterial(bool at_nodes)
{
  if (_current_task == "add_material")
  {
    if (!parameters().hasDefaultCoupledValue("temperature"))
      mooseError(
          "Attempt to add a PorousFlowTemperature material without setting a temperature variable");

    std::string material_type = "PorousFlowTemperature";
    InputParameters params = _factory.getValidParams(material_type);

    params.applySpecificParameters(parameters(), {"temperature"});
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;

    std::string material_name = "PorousFlowActionBase_Temperature_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_Temperature";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addMassFractionMaterial(bool at_nodes)
{
  if (_current_task == "add_material")
  {
    if (!(parameters().hasDefaultCoupledValue("mass_fraction_vars") ||
          parameters().hasCoupledValue("mass_fraction_vars")))
      mooseError("Attempt to add a PorousFlowMassFraction material without setting the "
                 "mass_fraction_vars");

    std::string material_type = "PorousFlowMassFraction";
    InputParameters params = _factory.getValidParams(material_type);

    params.applySpecificParameters(parameters(), {"mass_fraction_vars"});
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;

    std::string material_name = "PorousFlowActionBase_MassFraction_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_MassFraction";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addEffectiveFluidPressureMaterial(bool at_nodes)
{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowEffectiveFluidPressure";
    InputParameters params = _factory.getValidParams(material_type);

    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;

    std::string material_name = "PorousFlowUnsaturated_EffectiveFluidPressure_qp";
    if (at_nodes)
      material_name = "PorousFlowUnsaturated_EffectiveFluidPressure";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addVolumetricStrainMaterial(const std::vector<VariableName> & displacements,
                                                  bool consistent_with_displaced_mesh)
{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowVolumetricStrain";
    InputParameters params = _factory.getValidParams(material_type);

    std::string material_name = "PorousFlowActionBase_VolumetricStrain";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("displacements") = displacements;
    params.set<bool>("consistent_with_displaced_mesh") = consistent_with_displaced_mesh;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addSingleComponentFluidMaterial(bool at_nodes,
                                                      unsigned phase,
                                                      bool compute_density_and_viscosity,
                                                      bool compute_internal_energy,
                                                      bool compute_enthalpy,
                                                      const UserObjectName & fp)
{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowSingleComponentFluid";
    InputParameters params = _factory.getValidParams(material_type);

    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned int>("phase") = phase;
    params.set<bool>("compute_density_and_viscosity") = compute_density_and_viscosity;
    params.set<bool>("compute_internal_energy") = compute_internal_energy;
    params.set<bool>("compute_enthalpy") = compute_enthalpy;
    params.set<UserObjectName>("fp") = fp;

    std::string material_name = "PorousFlowActionBase_FluidProperties_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_FluidProperties";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addBrineMaterial(VariableName nacl_brine,
                                       bool at_nodes,
                                       unsigned phase,
                                       bool compute_density_and_viscosity,
                                       bool compute_internal_energy,
                                       bool compute_enthalpy)
{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowBrine";
    InputParameters params = _factory.getValidParams(material_type);

    params.set<std::vector<VariableName>>("xnacl") = {nacl_brine};
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned int>("phase") = phase;
    params.set<bool>("compute_density_and_viscosity") = compute_density_and_viscosity;
    params.set<bool>("compute_internal_energy") = compute_internal_energy;
    params.set<bool>("compute_enthalpy") = compute_enthalpy;

    std::string material_name = "PorousFlowActionBase_FluidProperties_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_FluidProperties";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addRelativePermeabilityCorey(
    bool at_nodes, unsigned phase, Real n, Real s_res, Real sum_s_res)

{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowRelativePermeabilityCorey";
    InputParameters params = _factory.getValidParams(material_type);

    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<Real>("n") = n;
    params.set<unsigned int>("phase") = phase;
    params.set<Real>("s_res") = s_res;
    params.set<Real>("sum_s_res") = sum_s_res;

    std::string material_name = "PorousFlowActionBase_RelativePermeability_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_RelativePermeability_nodal";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addRelativePermeabilityFLAC(
    bool at_nodes, unsigned phase, Real m, Real s_res, Real sum_s_res)
{
  if (_current_task == "add_material")
  {
    std::string material_type = "PorousFlowRelativePermeabilityFLAC";
    InputParameters params = _factory.getValidParams(material_type);

    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<Real>("m") = m;
    params.set<unsigned int>("phase") = phase;
    params.set<Real>("s_res") = s_res;
    params.set<Real>("sum_s_res") = sum_s_res;

    std::string material_name = "PorousFlowActionBase_RelativePermeability_qp";
    if (at_nodes)
      material_name = "PorousFlowActionBase_RelativePermeability_nodal";

    params.set<bool>("at_nodes") = at_nodes;
    _problem->addMaterial(material_type, material_name, params);
  }
}

void
PorousFlowActionBase::addCapillaryPressureVG(Real m, Real alpha, std::string userobject_name)
{
  if (_current_task == "add_user_object")
  {
    std::string userobject_type = "PorousFlowCapillaryPressureVG";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<Real>("m") = m;
    params.set<Real>("alpha") = alpha;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorSaturated(unsigned phase,
                                                          bool multiply_by_density,
                                                          std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type = "PorousFlowAdvectiveFluxCalculatorSaturated";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorUnsaturated(unsigned phase,
                                                            bool multiply_by_density,
                                                            std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type = "PorousFlowAdvectiveFluxCalculatorUnsaturated";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorSaturatedHeat(unsigned phase,
                                                              bool multiply_by_density,
                                                              std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type = "PorousFlowAdvectiveFluxCalculatorSaturatedHeat";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorUnsaturatedHeat(unsigned phase,
                                                                bool multiply_by_density,
                                                                std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type = "PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorSaturatedMultiComponent(unsigned phase,
                                                                        unsigned fluid_component,
                                                                        bool multiply_by_density,
                                                                        std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type = "PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    params.set<unsigned>("fluid_component") = fluid_component;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}

void
PorousFlowActionBase::addAdvectiveFluxCalculatorUnsaturatedMultiComponent(
    unsigned phase, unsigned fluid_component, bool multiply_by_density, std::string userobject_name)
{
  if (_stabilization == StabilizationEnum::KT && _current_task == "add_user_object")
  {
    const std::string userobject_type =
        "PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent";
    InputParameters params = _factory.getValidParams(userobject_type);
    params.set<MooseEnum>("flux_limiter_type") = _flux_limiter_type;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<unsigned>("phase") = phase;
    params.set<bool>("multiply_by_density") = multiply_by_density;
    params.set<unsigned>("fluid_component") = fluid_component;
    _problem->addUserObject(userobject_type, userobject_name, params);
  }
}
