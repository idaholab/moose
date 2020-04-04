//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"
#include "libmesh/libmesh.h"

/**
 * Scope guard for starting and stopping Floating Point Exception Trapping
 */
class FloatingPointExceptionGuard
{
public:
  /**
   * Instantiation turns on FPE Trapping as long as trapping is enabled on the application. By
   * default trapping is enabled when the DEBUG symbol is defined (dbg mode), but can be overridden
   * and turned on and off for any particular simulation run.
   */
  FloatingPointExceptionGuard(const MooseApp & moose_app)
    : _trapping_enabled(moose_app.getFPTrapFlag())
  {
    if (_trapping_enabled)
      libMesh::enableFPE(true);
  }

  /**
   * Stop FPE Trapping on destruction
   */
  ~FloatingPointExceptionGuard()
  {
    if (_trapping_enabled)
      libMesh::enableFPE(false);
  }

private:
  /// Determine whether or not PFE trapping needs to be toggled off.
  const bool _trapping_enabled;
};
