//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealVectorValueAux.h"

template <>
InputParameters
validParams<MaterialRealVectorValueAux>()
{
  InputParameters params = validParams<MaterialAuxBase<>>();
  params.addParam<unsigned int>("component", 0, "The vector component to consider for this kernel");

  return params;
}

MaterialRealVectorValueAux::MaterialRealVectorValueAux(const InputParameters & parameters)
  : MaterialAuxBase<RealVectorValue>(parameters), _component(getParam<unsigned int>("component"))
{
  if (_component > LIBMESH_DIM)
    mooseError(
        "The component ", _component, " does not exist for ", LIBMESH_DIM, " dimensional problems");
}

Real
MaterialRealVectorValueAux::getRealValue()
{
  return _prop[_qp](_component);
}
