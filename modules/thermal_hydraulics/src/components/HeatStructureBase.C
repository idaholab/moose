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
  return params;
}

HeatStructureBase::HeatStructureBase(const InputParameters & params)
  : Component2D(params),
    HeatStructureInterface(this),
    _number_of_hs(_n_regions)
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
