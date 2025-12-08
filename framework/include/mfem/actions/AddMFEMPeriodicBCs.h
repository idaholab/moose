//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "Action.h"
#include "MooseObject.h"

// add a struct to serve as the type for the input file parsing
class MFEMPeriodicByVector : public MooseObject
{
public:
  static InputParameters validParams();
  MFEMPeriodicByVector(const InputParameters& parameters);
};

class AddMFEMPeriodicBCs : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMPeriodicBCs(const InputParameters & parameters);

  virtual void act() override;
};

#endif
