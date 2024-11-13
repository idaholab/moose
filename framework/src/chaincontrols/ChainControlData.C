//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControlData.h"

const ChainControl &
ChainControlDataBase::getChainControl() const
{
  if (!_chain_control)
    mooseError("No chain control has been set for chain control data '", name(), "'.");

  return *_chain_control;
}
