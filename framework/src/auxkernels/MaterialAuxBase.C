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

// MOOSE includes
#include "MaterialAuxBase.h"

template <>
InputParameters
validParams<MaterialAuxBase<>>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<MaterialPropertyName>("property", "The scalar material property name");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>(
      "offset", 0, "The offset to add to your material property for visualization");
  return params;
}
