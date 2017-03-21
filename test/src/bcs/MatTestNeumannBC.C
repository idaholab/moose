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

#include "MatTestNeumannBC.h"

template <>
InputParameters
validParams<MatTestNeumannBC>()
{
  InputParameters p = validParams<NeumannBC>();
  p.addRequiredParam<std::string>("mat_prop",
                                  "The material property that gives the value of the BC");
  return p;
}

MatTestNeumannBC::MatTestNeumannBC(const InputParameters & parameters)
  : NeumannBC(parameters), _prop_name(getParam<std::string>("mat_prop"))
{
  if (hasBoundaryMaterialProperty<Real>(_prop_name))
    _value = &getMaterialPropertyByName<Real>(_prop_name);

  else
    mooseError(
        "The material property ", _prop_name, " is not defined on all boundaries of this object");
}

Real
MatTestNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * (*_value)[_qp];
}
