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

#include "MaterialEigenKernel.h"

template <>
InputParameters
validParams<MaterialEigenKernel>()
{
  InputParameters params = validParams<EigenKernel>();
  params.addParam<MaterialPropertyName>("mat", "Material property name (pseudo-stateful)");
  return params;
}

MaterialEigenKernel::MaterialEigenKernel(const InputParameters & parameters)
  : EigenKernel(parameters),
    _propname(getParam<MaterialPropertyName>("mat")),
    _mat(_is_implicit ? getMaterialPropertyByName<Real>(_propname)
                      : getMaterialPropertyByName<Real>(_propname + "_old"))
{
}

Real
MaterialEigenKernel::computeQpResidual()
{
  return -_mat[_qp] * _test[_i][_qp];
}
