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
#include "ADMatDiffusion.h"

template<>
InputParameters validParams<ADMatDiffusion>()
{
  InputParameters params = validParams<ADKernel>();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>("prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}


ADMatDiffusion::ADMatDiffusion(const InputParameters & parameters) :
    ADKernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = &getMaterialProperty<ADReal>("prop_name");
  else if (prop_state == "old")
    _diff = &getMaterialPropertyOld<ADReal>("prop_name");
  else if (prop_state == "older")
    _diff = &getMaterialPropertyOlder<ADReal>("prop_name");
}

ADReal
ADMatDiffusion::computeQpResidual()
{
  return (*_diff)[_qp] * (_grad_test[_i][_qp] * _grad_u[_qp]);
}
