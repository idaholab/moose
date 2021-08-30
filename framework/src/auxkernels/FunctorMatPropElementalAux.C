//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorMatPropElementalAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorMatPropElementalAux);

InputParameters
FunctorMatPropElementalAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Evaluates a functor material property on the current element."
      "For finite volume, this evaluates the material property at the centroid.");
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The functor mat prop");
  return params;
}

FunctorMatPropElementalAux::FunctorMatPropElementalAux(const InputParameters & parameters)
  : AuxKernel(parameters), _mat_prop(getFunctorMaterialProperty<ADReal>("mat_prop"))
{
}

Real
FunctorMatPropElementalAux::computeValue()
{
  return MetaPhysicL::raw_value(_mat_prop(_current_elem));
}
