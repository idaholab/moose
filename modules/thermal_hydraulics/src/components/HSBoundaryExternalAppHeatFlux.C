//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryExternalAppHeatFlux.h"
#include "HeatStructureCylindricalBase.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", HSBoundaryExternalAppHeatFlux);

InputParameters
HSBoundaryExternalAppHeatFlux::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addRequiredParam<VariableName>(
      "heat_flux_name",
      "Name to give the heat flux variable transferred from the external application");
  params.addParam<bool>(
      "heat_flux_is_monomial",
      true,
      "If true, makes the heat flux variable transferred from the external application to have the "
      "FE type 'CONSTANT MONOMIAL'. Else, the FE type is 'FIRST LAGRANGE'.");
  params.addRequiredParam<PostprocessorName>(
      "perimeter_ext", "Name to give the external application perimeter post-processor");
  params.addRequiredParam<bool>(
      "heat_flux_is_inward",
      "Set to true if the transferred heat flux corresponds to the inward direction on the heat "
      "structure boundary; else the outward direction");

  params.addClassDescription("Heat structure boundary condition to apply a heat flux transferred "
                             "from another application.");

  return params;
}

HSBoundaryExternalAppHeatFlux::HSBoundaryExternalAppHeatFlux(const InputParameters & params)
  : HSBoundary(params),
    _heat_flux_name(getParam<VariableName>("heat_flux_name")),
    _perimeter_ext_pp_name(getParam<PostprocessorName>("perimeter_ext"))
{
}

void
HSBoundaryExternalAppHeatFlux::check() const
{
  HSBoundary::check();

  // HeatStructurePlate and HeatStructureFromFile3D are not yet supported
  checkComponentOfTypeExistsByName<HeatStructureCylindricalBase>(_hs_name);

  // Check that all boundaries are of type INNER or OUTER
  if (hasComponentByName<Component2D>(_hs_name))
  {
    checkAllComponent2DBoundariesAreExternal();
    if (allComponent2DBoundariesAreExternal())
      if (hasCommonComponent2DExternalBoundaryType())
      {
        const auto boundary_type = getCommonComponent2DExternalBoundaryType();
        if (boundary_type != Component2D::ExternalBoundaryType::INNER &&
            boundary_type != Component2D::ExternalBoundaryType::OUTER)
          logError("The boundaries in 'boundary' must be of an inner/outer type, not of a "
                   "start/end type.");
      }
  }
}

void
HSBoundaryExternalAppHeatFlux::addVariables()
{
  const HeatStructureInterface & hs = getComponentByName<HeatStructureInterface>(_hs_name);
  const std::vector<SubdomainName> & subdomain_names =
      hs.getGeometricalComponent().getSubdomainNames();

  const auto fe_type = getParam<bool>("heat_flux_is_monomial") ? libMesh::FEType(CONSTANT, MONOMIAL)
                                                               : libMesh::FEType(FIRST, LAGRANGE);
  getTHMProblem().addSimVariable(false, _heat_flux_name, fe_type, subdomain_names);
}

void
HSBoundaryExternalAppHeatFlux::addMooseObjects()
{
  // Add external perimeter PP
  {
    const std::string class_name = "Receiver";
    InputParameters params = _factory.getValidParams(class_name);
    getTHMProblem().addPostprocessor(class_name, _perimeter_ext_pp_name, params);
  }

  // Add BC
  {
    const std::string class_name = "FunctorNeumannBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary;
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<MooseFunctorName>("functor") = _heat_flux_name;
    params.set<MooseFunctorName>("coefficient") = _perimeter_ext_pp_name;
    params.set<bool>("flux_is_inward") = getParam<bool>("heat_flux_is_inward");
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "bc"), params);
  }

  // Add scale function to use as 'prefactor' in heat rate PP
  const FunctionName scale_fn_name = genName(name(), "scale_fn");
  {
    const std::string class_name = "ParsedFunction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::string>("expression") = "sign * P";
    params.set<std::vector<std::string>>("symbol_names") = {"sign", "P"};
    if (getParam<bool>("heat_flux_is_inward"))
      params.set<std::vector<std::string>>("symbol_values") = {"1", _perimeter_ext_pp_name};
    else
      params.set<std::vector<std::string>>("symbol_values") = {"-1", _perimeter_ext_pp_name};
    getTHMProblem().addFunction(class_name, scale_fn_name, params);
  }

  // Add heat rate PP
  {
    const std::string class_name = "SideIntegralFunctorPostprocessor";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary;
    params.set<MooseFunctorName>("functor") = _heat_flux_name;
    params.set<MooseFunctorName>("prefactor") = scale_fn_name;
    params.set<MooseEnum>("functor_argument") = "qp";
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, genSafeName(name(), "integral"), params);
  }
}
