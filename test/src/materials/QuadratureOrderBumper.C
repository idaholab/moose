//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadratureOrderBumper.h"

registerMooseObject("MooseTestApp", QuadratureOrderBumper);

InputParameters
QuadratureOrderBumper::validParams()
{
  MooseEnum order("AUTO CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
                  "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
                  "EIGHTTEENTH NINTEENTH TWENTIETH",
                  "AUTO");

  InputParameters params = Material::validParams();
  params.addParam<MooseEnum>("order", order, "Order of the quadrature");
  return params;
}

QuadratureOrderBumper::QuadratureOrderBumper(const InputParameters & parameters)
  : Material(parameters)
{
  for (auto block : blockIDs())
    _fe_problem.bumpVolumeQRuleOrder(Moose::stringToEnum<Order>(getParam<MooseEnum>("order")),
                                     block);
}
