/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
