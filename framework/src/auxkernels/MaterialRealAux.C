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

#include "MaterialRealAux.h"

template <>
InputParameters
validParams<MaterialRealAux>()
{
  InputParameters params = validParams<MaterialAuxBase<Real>>();
  return params;
}

MaterialRealAux::MaterialRealAux(const InputParameters & parameters)
  : MaterialAuxBase<Real>(parameters)
{
}

Real
MaterialRealAux::getRealValue()
{
  return _prop[_qp];
}
