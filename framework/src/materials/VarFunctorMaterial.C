//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VarFunctorMaterial.h"
#include "MooseVariableFV.h"
#include "FunctorMaterialProperty.h"

registerMooseObject("MooseApp", VarFunctorMaterial);

InputParameters
VarFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.addRequiredParam<MooseFunctorName>("var", "The field variable to be coupled in");
  params.addRequiredParam<MaterialPropertyName>("mat_prop_name",
                                                "The name of the material property to produce");
  params.addClassDescription("Creates a functor material property whose evaluation corresponds to "
                             "the evaluation of the coupled variable at a given location");
  return params;
}

VarFunctorMaterial::VarFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _var(getFunctor<ADReal>("var")),
    _functor_prop(declareFunctorProperty<ADReal>("mat_prop_name"))
{
  _functor_prop.setFunctor(
      _mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal { return _var(r, t); });
  _functor_prop.setCacheClearanceSchedule(
      std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
}
