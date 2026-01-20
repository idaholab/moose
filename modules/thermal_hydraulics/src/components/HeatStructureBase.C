//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureBase.h"
#include "ConstantFunction.h"

InputParameters
HeatStructureBase::validParams()
{
  InputParameters params = Component2D::validParams();
  params += HeatStructureInterface::validParams();

  params.addParam<Real>("T_rel_step_tol", 1e-5, "Temperature relative step tolerance");
  params.addParam<Real>("res_tol", 1e-5, "Residual tolerance");

  return params;
}

HeatStructureBase::HeatStructureBase(const InputParameters & params)
  : Component2D(params), HeatStructureInterface(this), _number_of_hs(_n_regions)
{
}

void
HeatStructureBase::init()
{
  Component2D::init();
  HeatStructureInterface::init();
}

void
HeatStructureBase::check() const
{
  Component2D::check();
  HeatStructureInterface::check();
}

const unsigned int &
HeatStructureBase::getIndexFromName(const std::string & name) const
{
  return _name_index.at(name);
}

bool
HeatStructureBase::usingSecondOrderMesh() const
{
  return HeatConductionModel::feType().order == SECOND;
}

Convergence *
HeatStructureBase::getNonlinearConvergence() const
{
  if (!isParamValid("solid_properties"))
    mooseError("ComponentsConvergence may only be used if 'solid_properties' is provided.");

  return &getTHMProblem().getConvergence(nonlinearConvergenceName());
}

void
HeatStructureBase::addVariables()
{
  HeatStructureInterface::addVariables();
}

void
HeatStructureBase::addMooseObjects()
{
  HeatStructureInterface::addMooseObjects();

  if (isParamValid("solid_properties"))
  {
    const auto sp_names = getParam<std::vector<UserObjectName>>("solid_properties");
    const auto T_ref = getParam<std::vector<Real>>("solid_properties_T_ref");
    for (unsigned int i = 0; i < sp_names.size(); i++)
      addConstantDensitySolidPropertiesMaterial(sp_names[i], T_ref[i], i);

    // add objects needed for ComponentsConvergence
    addNonlinearStepFunctorMaterial(HeatConductionModel::TEMPERATURE, "hs_T_step", true);
    const auto & blocks = getSubdomainNames();
    for (const auto i : make_range(blocks.size()))
    {
      addAverageElementSizePostprocessor(blocks[i]);
      addMaximumFunctorPostprocessor("hs_T_step", blocks[i] + "_T_step", T_ref[i], {blocks[i]});
      addResidualNormPostprocessor(blocks[i], sp_names[i], T_ref[i]);
    }

    std::vector<PostprocessorName> pp_names;
    std::vector<std::string> descriptions;
    std::vector<Real> tolerances;
    for (const auto i : make_range(blocks.size()))
    {
      pp_names.push_back(blocks[i] + "_T_step");
      descriptions.push_back("T step (" + blocks[i] + ")");
      tolerances.push_back(getParam<Real>("T_rel_step_tol"));
    }
    for (const auto i : make_range(blocks.size()))
    {
      pp_names.push_back(blocks[i] + "_res");
      descriptions.push_back("residual (" + blocks[i] + ")");
      tolerances.push_back(getParam<Real>("res_tol"));
    }
    addMultiPostprocessorConvergence(pp_names, descriptions, tolerances);
  }
}

void
HeatStructureBase::addConstantDensitySolidPropertiesMaterial(const UserObjectName & sp_name,
                                                             const Real & T_ref,
                                                             unsigned int i_region) const
{
  const auto blocks = getSubdomainNames();
  const auto region_names = getNames();

  const std::string class_name = "ADConstantDensityThermalSolidPropertiesMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = {blocks[i_region]};
  params.set<std::vector<VariableName>>("temperature") = {HeatConductionModel::TEMPERATURE};
  params.set<UserObjectName>("sp") = sp_name;
  params.set<Real>("T_ref") = T_ref;
  getTHMProblem().addMaterial(
      class_name, genName(name(), class_name, region_names[i_region]), params);
}

void
HeatStructureBase::addAverageElementSizePostprocessor(const SubdomainName & block)
{
  const std::string class_name = "AverageElementSize";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = {block};
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  getTHMProblem().addPostprocessor(class_name, block + "_havg", params);
}

void
HeatStructureBase::addResidualNormPostprocessor(const SubdomainName & block,
                                                const UserObjectName & sp_name,
                                                Real T_ref)
{
  const std::string class_name = "NormalizedHeatStructureResidualNorm";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = HeatConductionModel::TEMPERATURE;
  params.set<std::vector<SubdomainName>>("block") = {block};
  params.set<MooseEnum>("norm_type") = "l_inf";
  params.set<Real>("T_ref") = T_ref;
  params.set<UserObjectName>("solid_properties") = sp_name;
  params.set<PostprocessorName>("ref_elem_size") = block + "_havg";
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR_CONVERGENCE;
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  getTHMProblem().addPostprocessor(class_name, block + "_res", params);
}
