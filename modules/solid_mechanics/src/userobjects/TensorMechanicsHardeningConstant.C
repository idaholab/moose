//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsHardeningConstant.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsHardeningConstant);

InputParameters
TensorMechanicsHardeningConstant::validParams()
{
  InputParameters params = TensorMechanicsHardeningModel::validParams();
  params.addParam<Real>("value",
                        1.0,
                        "The value of the parameter for all internal parameter.  "
                        "This is perfect plasticity - there is no hardening.");
  params.addParam<bool>("convert_to_radians",
                        false,
                        "If true, the value you entered will be "
                        "multiplied by Pi/180 before passing to the "
                        "Plasticity algorithms");
  params.addClassDescription(
      "No hardening - the parameter is independent of the internal parameter(s)");
  return params;
}

TensorMechanicsHardeningConstant::TensorMechanicsHardeningConstant(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val(getParam<bool>("convert_to_radians") ? getParam<Real>("value") * libMesh::pi / 180.0
                                              : getParam<Real>("value"))
{
}

Real TensorMechanicsHardeningConstant::value(Real /*intnl*/) const { return _val; }

Real TensorMechanicsHardeningConstant::derivative(Real /*intnl*/) const { return 0.0; }

std::string
TensorMechanicsHardeningConstant::modelName() const
{
  return "Constant";
}
