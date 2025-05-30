//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsHardeningConstant.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsHardeningConstant);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsHardeningConstant,
                           "01/01/2025 00:00",
                           SolidMechanicsHardeningConstant);

InputParameters
SolidMechanicsHardeningConstant::validParams()
{
  InputParameters params = SolidMechanicsHardeningModel::validParams();
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

SolidMechanicsHardeningConstant::SolidMechanicsHardeningConstant(const InputParameters & parameters)
  : SolidMechanicsHardeningModel(parameters),
    _val(getParam<bool>("convert_to_radians") ? getParam<Real>("value") * libMesh::pi / 180.0
                                              : getParam<Real>("value"))
{
}

Real
SolidMechanicsHardeningConstant::value(Real /*intnl*/) const
{
  return _val;
}

Real
SolidMechanicsHardeningConstant::derivative(Real /*intnl*/) const
{
  return 0.0;
}

std::string
SolidMechanicsHardeningConstant::modelName() const
{
  return "Constant";
}
