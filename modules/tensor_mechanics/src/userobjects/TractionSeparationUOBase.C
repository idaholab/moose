//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TractionSeparationUOBase.h"
#include "Material.h"
#include "MooseError.h"

template <>
InputParameters
validParams<TractionSeparationUOBase>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addClassDescription(
      "User Object implementing basic functions for traction separations law");
  params.set<ExecFlagEnum>("execute_on") = EXEC_CUSTOM;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.addParam<std::vector<std::string>>(
      "cohesive_law_stateful_properties_names",
      std::vector<std::string>({}),
      "This vector contains the name of the stateful cohesive law variables.");

  return params;
}

TractionSeparationUOBase::TractionSeparationUOBase(const InputParameters & parameters)
  : SideUserObject(parameters),
    _JumpLocal(getMaterialPropertyByName<RealVectorValue>("JumpLocal")),
    _cohesive_law_stateful_properties_names(
        getParam<std::vector<std::string>>("cohesive_law_stateful_properties_names"))
{
}

void
TractionSeparationUOBase::statefulMaterialPropertyNames(
    std::vector<std::string> & materialPropertyNames) const
{
  unsigned int num_prop = _cohesive_law_stateful_properties_names.size();
  materialPropertyNames.resize(num_prop);

  for (unsigned int i = 0; i < num_prop; i++)
    materialPropertyNames[i] = _cohesive_law_stateful_properties_names[i];
}

unsigned int
TractionSeparationUOBase::statefulMaterialPropertySize(unsigned int /*materialPropertyID*/) const
{
  mooseError("TractionSeparationUOBase::statefulMaterialPropertySize should never "
             "be called directly but always subclassed");
}

void
TractionSeparationUOBase::initStatefulMaterialProperty(
    unsigned int /*materialPropertyID*/, std::vector<Real> & /*statefulePropertyValue*/) const
{
  mooseError("TractionSeparationUOBase::initStatefulMaterialProperty should never "
             "be called directly but always subclassed");
}

void
TractionSeparationUOBase::updateStatefulMaterialProperty(
    unsigned int /*qp*/,
    unsigned int /*materialPropertyID*/,
    std::vector<Real> & /*statefulePropertyValue*/,
    const std::vector<Real> & /*statefulePropertyValue_old*/) const
{

  mooseError("TractionSeparationUOBase::updateStatefulMaterialProperty should never "
             "be called directly but always subclassed");
}

void
TractionSeparationUOBase::computeTractionLocal(unsigned int /*qp*/,
                                               RealVectorValue & /*TractionLocal*/) const
{

  mooseError("TractionSeparationUOBase::computeTractionLocal should never "
             "be called directly but always subclassed");
}

void
TractionSeparationUOBase::computeTractionSpatialDerivativeLocal(
    unsigned int /*qp*/, RankTwoTensor & /*TractionDerivativeLocal*/) const
{

  mooseError("TractionSeparationUOBase::computeTractionLocal should never "
             "be called directly but always subclassed");
}

// ovverride standard UO functions
void
TractionSeparationUOBase::initialize()
{
}

void
TractionSeparationUOBase::execute()
{
  mooseError("execute TractionSeparationUOBase must be called explicitly from Materials");
}

void
TractionSeparationUOBase::finalize()
{
  mooseError("finalize TractionSeparationUOBase must be called explicitly from Materials");
}

void
TractionSeparationUOBase::threadJoin(const UserObject &)
{
  mooseError("threadJoin TractionSeparationUOBase must be called explicitly from Materials");
}
