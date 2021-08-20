//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVVarFunctorMaterial.h"
#include "MooseVariableFV.h"
#include "FunctorMaterialProperty.h"

registerMooseObject("MooseApp", FVVarFunctorMaterial);

InputParameters
FVVarFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredCoupledVar("var", "The finite volume variable to be coupled in");
  params.addRequiredParam<MaterialPropertyName>("mat_prop_name",
                                                "The name of the material property to produce");
  params.addClassDescription("Creates a functor material property whose evaluation corresponds to "
                             "the evaluation of the coupled variable at a given location");
  return params;
}

FVVarFunctorMaterial::FVVarFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _var(*getVarHelper<MooseVariableFV<Real>>("var", 0)),
    _functor_prop(declareFunctorProperty<ADReal>("mat_prop_name"))
{
  _functor_prop.setFunctor(
      _mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal { return _var(geom_quantity); });
}
