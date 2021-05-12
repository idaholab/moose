//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

/**
 * Surrogate BC object for setting up enthalphy injection BC
 *
 * It specifies paramteres that users have to provide like in a normal object,
 * but does not do anything.  PorousFlowAddBCAction sets up the concrete objects
 * that will model this type of BC.
 */
class PorousFlowSinkBC : public MooseObject
{
public:
  static InputParameters validParams();
  static InputParameters validParamsCommon();

  PorousFlowSinkBC(const InputParameters & parameters);
};
