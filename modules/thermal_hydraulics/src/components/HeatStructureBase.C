//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureBase.h"
#include "SolidMaterialProperties.h"
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
  : Component2D(params),
    HeatStructureInterface(this),
    _number_of_hs(_n_regions),
    _nl_conv_name(genName(name(), "nlconv"))
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

  return &getTHMProblem().getConvergence(_nl_conv_name);
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

  if (isParamValid("materials"))
  {
    _hc_model->addMaterials();

    for (unsigned int i = 0; i < _n_regions; i++)
    {
      const SolidMaterialProperties & smp =
          getTHMProblem().getUserObject<SolidMaterialProperties>(_material_names[i]);

      Component * comp = (_parent != nullptr) ? _parent : this;
      // if the values were given as constant, allow them to be controlled
      const ConstantFunction * k_fn = dynamic_cast<const ConstantFunction *>(&smp.getKFunction());
      if (k_fn != nullptr)
        comp->connectObject(k_fn->parameters(), k_fn->name(), "k", "value");

      const ConstantFunction * cp_fn = dynamic_cast<const ConstantFunction *>(&smp.getCpFunction());
      if (cp_fn != nullptr)
        comp->connectObject(cp_fn->parameters(), cp_fn->name(), "cp", "value");

      const ConstantFunction * rho_fn =
          dynamic_cast<const ConstantFunction *>(&smp.getRhoFunction());
      if (rho_fn != nullptr)
        comp->connectObject(rho_fn->parameters(), rho_fn->name(), "rho", "value");
    }
  }

  if (isParamValid("solid_properties"))
  {
    const auto sp_names = getParam<std::vector<UserObjectName>>("solid_properties");
    const auto T_ref = getParam<std::vector<Real>>("solid_properties_T_ref");
    for (unsigned int i = 0; i < sp_names.size(); i++)
      addConstantDensitySolidPropertiesMaterial(sp_names[i], T_ref[i], i);

    // add objects needed for ComponentsConvergence
    addTemperatureStepFunctorMaterial();
    const auto & blocks = getSubdomainNames();
    for (const auto i : make_range(blocks.size()))
    {
      addAverageElementSizePostprocessor(blocks[i]);
      addTemperatureStepPostprocessor(blocks[i], T_ref[i]);
      addResidualNormPostprocessor(blocks[i], sp_names[i], T_ref[i]);
    }
    addNonlinearConvergence();
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
HeatStructureBase::addTemperatureStepFunctorMaterial()
{
  const std::string class_name = "ADFunctorChangeFunctorMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<MooseFunctorName>("functor") = HeatConductionModel::TEMPERATURE;
  params.set<MooseEnum>("change_over") = "nonlinear";
  params.set<std::string>("prop_name") = "hs_T_change";
  params.set<bool>("take_absolute_value") = true;
  getTHMProblem().addFunctorMaterial(class_name, genName(name(), "Tstep_fmat"), params);
}

void
HeatStructureBase::addTemperatureStepPostprocessor(const SubdomainName & block, Real T_ref)
{
  const std::string class_name = "ElementExtremeFunctorValue";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = {block};
  params.set<MooseEnum>("value_type") = "max";
  params.set<MooseFunctorName>("functor") = "hs_T_change";
  params.set<Real>("scale") = 1.0 / T_ref;
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR_CONVERGENCE;
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  getTHMProblem().addPostprocessor(class_name, block + "_Tstep", params);
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

void
HeatStructureBase::addNonlinearConvergence()
{
  std::vector<PostprocessorName> T_rel_step, res;
  for (const auto & block : getSubdomainNames())
  {
    T_rel_step.push_back(block + "_Tstep");
    res.push_back(block + "_res");
  }

  const std::string class_name = "HeatStructureConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("blocks") = getSubdomainNames();
  params.set<std::vector<PostprocessorName>>("T_rel_step") = T_rel_step;
  params.set<std::vector<PostprocessorName>>("res") = res;
  params.applyParameters(parameters());
  getTHMProblem().addConvergence(class_name, _nl_conv_name, params);
}
