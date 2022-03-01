//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialStdVectorRealGradientAux.h"

registerMooseObject("MooseApp", MaterialStdVectorRealGradientAux);

InputParameters
MaterialStdVectorRealGradientAux::validParams()
{
  InputParameters params = MaterialStdVectorAuxBase<>::validParams();
  params.addClassDescription("Extracts a component of a material's std::vector<RealGradient> to an "
                             "aux variable.  If the std::vector is not of sufficient size then "
                             "zero is returned");
  params.addParam<unsigned int>(
      "component", 0, "The gradient component to be extracted for this kernel");
  return params;
}

MaterialStdVectorRealGradientAux::MaterialStdVectorRealGradientAux(
    const InputParameters & parameters)
  : MaterialStdVectorAuxBase<RealGradient>(parameters),
    _component(getParam<unsigned int>("component"))
{
  if (_component > LIBMESH_DIM)
    mooseError(
        "The component ", _component, " does not exist for ", LIBMESH_DIM, " dimensional problems");
}

Real
MaterialStdVectorRealGradientAux::getRealValue()
{
  return _prop[_qp][_index](_component);
}
