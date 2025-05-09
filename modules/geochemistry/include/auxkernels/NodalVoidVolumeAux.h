//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalVoidVolume.h"
#include "AuxKernel.h"

/**
 * AuxKernel to extract information from a NodalVoidVolume UserObject to record into an AuxVariable
 */
class NodalVoidVolumeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NodalVoidVolumeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const NodalVoidVolume & _nvv;
};
