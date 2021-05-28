//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class WaveSpeedVPP;
class HLLCUserObject;

/**
 * Gets wave speeds from HLLC user object
 */
class WaveSpeedVPP : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  WaveSpeedVPP(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  const HLLCUserObject & _hllc;
  const unsigned int _side_id;
  const Elem * _elem;
  VectorPostprocessorValue & _wave_speeds;
};
