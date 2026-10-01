//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SegregatedTestComponent.h"

#include "libmesh/fe_type.h"

registerMooseObject("ThermalHydraulicsTestApp", SegregatedTestComponent);

InputParameters
SegregatedTestComponent::validParams()
{
  InputParameters params = Component1D::validParams();
  params.addParam<SolverSystemName>(
      "u_solver_system", "nl0", "Solver system to which the driving variable 'u' is added");
  params.addParam<SolverSystemName>(
      "v_solver_system", "nl0", "Solver system to which the coupled variable 'v' is added");
  params.addClassDescription(
      "Test component that solves two coupled Poisson equations in separate solver systems.");
  return params;
}

SegregatedTestComponent::SegregatedTestComponent(const InputParameters & parameters)
  : Component1D(parameters),
    _u_var_name(genSafeName(name(), "u")),
    _v_var_name(genSafeName(name(), "v")),
    _u_solver_system(getParam<SolverSystemName>("u_solver_system")),
    _v_solver_system(getParam<SolverSystemName>("v_solver_system"))
{
}

void
SegregatedTestComponent::addVariables()
{
  const auto & subdomains = getSubdomainNames();
  const FEType fe_type(FIRST, LAGRANGE);

  _sim.addSimVariable(true, _u_var_name, fe_type, subdomains, 1.0, _u_solver_system);
  _sim.addSimVariable(true, _v_var_name, fe_type, subdomains, 1.0, _v_solver_system);
}

void
SegregatedTestComponent::addMooseObjects()
{
  const auto & subdomains = getSubdomainNames();
  const BoundaryName in_boundary = genName(name(), "in");
  const BoundaryName out_boundary = genName(name(), "out");

  // -div(grad(u)) = 0
  {
    const std::string class_name = "Diffusion";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = _u_var_name;
    pars.set<std::vector<SubdomainName>>("block") = subdomains;
    _sim.addKernel(class_name, genName(name(), "u_diffusion"), pars);
  }

  // -div(grad(v)) = source
  {
    const std::string class_name = "Diffusion";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = _v_var_name;
    pars.set<std::vector<SubdomainName>>("block") = subdomains;
    _sim.addKernel(class_name, genName(name(), "v_diffusion"), pars);
  }

  // Source for 'v'
  {
    const std::string class_name = "CoupledForce";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = _v_var_name;
    pars.set<std::vector<VariableName>>("v") = {_u_var_name};
    pars.set<std::vector<SubdomainName>>("block") = subdomains;
    _sim.addKernel(class_name, genName(name(), "v_coupling"), pars);
  }

  // Boundary conditions
  auto add_dirichlet_bc =
      [&](const VariableName & var, const BoundaryName & boundary, Real value, const std::string & suffix)
  {
    const std::string class_name = "DirichletBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = var;
    pars.set<std::vector<BoundaryName>>("boundary") = {boundary};
    pars.set<Real>("value") = value;
    _sim.addBoundaryCondition(class_name, genName(name(), suffix), pars);
  };

  add_dirichlet_bc(_u_var_name, in_boundary, 0.0, "u_in_bc");
  add_dirichlet_bc(_u_var_name, out_boundary, 1.0, "u_out_bc");
  add_dirichlet_bc(_v_var_name, in_boundary, 0.0, "v_in_bc");
  add_dirichlet_bc(_v_var_name, out_boundary, 0.0, "v_out_bc");
}
