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

#include "MaterialStdVectorRealGradientAux.h"

template <>
InputParameters
validParams<MaterialStdVectorRealGradientAux>()
{
  InputParameters params = validParams<MaterialStdVectorAuxBase<>>();
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
