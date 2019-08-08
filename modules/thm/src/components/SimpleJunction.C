#include "SimpleJunction.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "THMIndices7Eqn.h"

registerMooseObject("THMApp", SimpleJunction);

template <>
InputParameters
validParams<SimpleJunction>()
{
  InputParameters params = validParams<FlowJunction>();
  params.addParam<Real>(
      "scaling_factor", 1.0, "Scaling factor for the Lagrange multiplier variable");
  return params;
}

SimpleJunction::SimpleJunction(const InputParameters & params)
  : FlowJunction(params),
    _lm_name(genName(name(), "lm")),
    _scaling_factor(getParam<Real>("scaling_factor"))
{
}

void
SimpleJunction::check() const
{
  FlowJunction::check();
}

void
SimpleJunction::addVariables()
{
  auto connected_subdomains = getConnectedSubdomainIDs();

  // add scalar variable (i.e. Lagrange multiplier)
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    _sim.addVariable(true, _lm_name, FEType(THIRD, SCALAR), connected_subdomains, _scaling_factor);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    _sim.addVariable(
        true, _lm_name, FEType(SEVENTH, SCALAR), connected_subdomains, _scaling_factor);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    unsigned int n_vars = THM7Eqn::N_EQ;
    auto fm_ncg = dynamic_cast<const FlowModelTwoPhaseNCG *>(_flow_model.get());
    if (fm_ncg != nullptr)
      n_vars += fm_ncg->getNCGSolutionVars().size();
    _sim.addVariable(true, _lm_name, FEType(n_vars, SCALAR), connected_subdomains, _scaling_factor);
  }
}

void
SimpleJunction::addMooseObjects()
{
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    add1Phase();
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    add2Phase();
}

void
SimpleJunction::add1Phase()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_pressure(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_enthalpy(1, FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_lambda(1, _lm_name);

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = EXEC_LINEAR;

  // add BC terms
  for (unsigned int i = 0; i < _boundary_names.size(); ++i)
  {
    Real vg = (i == 0) ? 1 : -1.; // this assumes i==0 is an inlet (1), and i==1 is an outlet (-1)

    // mass
    {
      std::string class_name = "OneDMassFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mass_bc"), params);
    }
    {
      std::string class_name = "OneDEqualValueConstraintBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 0;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mass_evc_bc"), params);
    }

    // momentum
    {
      std::string class_name = "OneDMomentumFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
      params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mom_bc"), params);
    }
    {
      std::string class_name = "OneDEqualValueConstraintBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 1;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mom_evc_bc"), params);
    }

    // energy
    {
      std::string class_name = "OneDEnergyFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
      params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("H") = cv_enthalpy;
      params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "erg_bc"), params);
    }
    {
      std::string class_name = "OneDEqualValueConstraintBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 2;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "erg_evc_bc"), params);
    }
  }

  // add the constraints
  {
    std::vector<VariableName> cv_var;
    cv_var.push_back(FlowModelSinglePhase::RHOA);
    cv_var.push_back(FlowModelSinglePhase::RHOUA);
    cv_var.push_back(FlowModelSinglePhase::RHOEA);

    InputParameters params = _factory.getValidParams("NodalEqualValueConstraint");
    params.set<NonlinearVariableName>("variable") = _lm_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<VariableName>>("var") = cv_var;
    _sim.addScalarKernel("NodalEqualValueConstraint", genName(name(), "ced"), params);
  }
}

void
SimpleJunction::add2Phase()
{
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);

  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_rho_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_pressure_liquid(1, FlowModelTwoPhase::PRESSURE_LIQUID);
  std::vector<VariableName> cv_enthalpy_liquid(1,
                                               FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_LIQUID);

  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_rho_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_pressure_vapor(1, FlowModelTwoPhase::PRESSURE_VAPOR);
  std::vector<VariableName> cv_enthalpy_vapor(1, FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_VAPOR);

  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_lambda(1, _lm_name);

  const TwoPhaseFluidProperties & tpfp = _sim.getUserObjectTempl<TwoPhaseFluidProperties>(_fp_name);
  UserObjectName fp_liquid = tpfp.getLiquidName();
  UserObjectName fp_vapor = tpfp.getVaporName();

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = EXEC_LINEAR;

  // add BC terms
  for (unsigned int i = 0; i < _boundary_names.size(); ++i)
  {
    Real vg = (i == 0) ? 1 : -1.; // this assumes i==0 is an inlet (1), and i==1 is an outlet (-1)

    // mass
    {
      std::string class_name = "OneDMassFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mass_bc_liquid"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 0;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "mass_evc_bc_liquid"), params);
    }

    // momentum
    {
      std::string class_name = "OneDMomentumFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<bool>("is_liquid") = true;
      // coupling
      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
      params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
      params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_LIQUID;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mom_bc_liquid"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 1;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "mom_evc_bc_liquid"), params);
    }

    // energy
    {
      std::string class_name = "OneDEnergyFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<bool>("is_liquid") = true;
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
      params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
      params.set<std::vector<VariableName>>("H") = cv_enthalpy_liquid;
      params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
      params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_LIQUID;
      params.set<std::vector<VariableName>>("A") = cv_area;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "erg_bc_liquid"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 2;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "erg_evc_bc_liquid"), params);
    }

    // mass
    {
      std::string class_name = "OneDMassFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mass_bc_vapor"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 3;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "mass_evc_bc_vapor"), params);
    }

    // momentum
    {
      std::string class_name = "OneDMomentumFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<bool>("is_liquid") = false;
      // coupling
      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
      params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
      params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_VAPOR;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "mom_bc_vapor"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 4;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "mom_evc_bc_vapor"), params);
    }

    // energy
    {
      std::string class_name = "OneDEnergyFreeBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<bool>("is_liquid") = false;
      params.set<Real>("normal") = _normals[i];
      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
      params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
      params.set<std::vector<VariableName>>("H") = cv_enthalpy_vapor;
      params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
      params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_VAPOR;
      params.set<std::vector<VariableName>>("A") = cv_area;
      _sim.addBoundaryCondition(class_name, genName(_boundary_names[i], "erg_bc_vapor"), params);
    }
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 5;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "erg_evc_bc_vapor"), params);
    }

    // volume fraction
    {
      InputParameters params = _factory.getValidParams("OneDEqualValueConstraintBC");
      params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<unsigned int>("component") = 6;
      params.set<Real>("vg") = vg;
      params.set<std::vector<VariableName>>("lambda") = cv_lambda;
      _sim.addBoundaryCondition(
          "OneDEqualValueConstraintBC", genName(_boundary_names[i], "vf_evc_bc_liquid"), params);
    }

    if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    {
      const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
      const std::vector<VariableName> & vars = fm.getNCGSolutionVars();
      unsigned int n_2phase_vars = THM7Eqn::N_EQ;

      for (std::size_t j = 0; j < vars.size(); j++)
      {
        {
          std::string class_name = "OneDMassNCGFreeBC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<NonlinearVariableName>("variable") = vars[j];
          params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
          params.set<Real>("normal") = _normals[i];
          params.set<std::vector<VariableName>>("arhoA") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
          params.set<std::vector<VariableName>>("arhouA") = {FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
          params.set<std::vector<VariableName>>("vel") = {FlowModelTwoPhase::VELOCITY_VAPOR};
          _sim.addBoundaryCondition(
              class_name, genName(_boundary_names[i], vars[j], "free_bc"), params);
        }
        {
          std::string class_name = "OneDEqualValueConstraintBC";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<NonlinearVariableName>("variable") = vars[j];
          params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
          params.set<unsigned int>("component") = n_2phase_vars + j;
          params.set<Real>("vg") = vg;
          params.set<std::vector<VariableName>>("lambda") = cv_lambda;
          _sim.addBoundaryCondition(
              class_name, genName(_boundary_names[i], vars[j], "evc_bc"), params);
        }
      }
    }
  }

  {
    std::vector<VariableName> cv_var = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                        FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                        FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                        FlowModelTwoPhase::ALPHA_RHO_A_VAPOR,
                                        FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR,
                                        FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR,
                                        FlowModelTwoPhase::BETA};

    if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    {
      const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
      const std::vector<VariableName> & vars = fm.getNCGSolutionVars();
      for (std::size_t i = 0; i < vars.size(); i++)
        cv_var.push_back(vars[i]);
    }

    InputParameters params = _factory.getValidParams("NodalEqualValueConstraint");
    params.set<NonlinearVariableName>("variable") = _lm_name;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<VariableName>>("var") = cv_var;
    _sim.addScalarKernel("NodalEqualValueConstraint", genName(name(), "ced"), params);
  }
}
