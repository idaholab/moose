//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarUserObject.h"

InputParameters
MortarUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  return params;
}

MortarUserObject::MortarUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    MortarConsumerInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, {_secondary_id}),
    NeighborCoupleable(this, /*nodal=*/false, /*neighbor_nodal=*/false, /*is_fv=*/false)
{
}
