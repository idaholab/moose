//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObjectBasePD.h"
#include "PeridynamicsMesh.h"

/**
 * Userobject class to ghost the required element for calculation on current processor from other
 * processors
 */
class GhostElemPD : public GeneralUserObjectBasePD
{
public:
  static InputParameters validParams();

  GhostElemPD(const InputParameters & parameters);

  virtual void meshChanged() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /**
   * Function to ghost/copy element information from other processors
   */
  void ghostElements();
};
