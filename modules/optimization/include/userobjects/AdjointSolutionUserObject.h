//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolutionUserObject.h"

class AdjointSolutionUserObject : public SolutionUserObject
{
public:
  static InputParameters validParams();

  AdjointSolutionUserObject(const InputParameters & parameters);

  /**
   * Skipping parent class initialSetup since it will be called in timestepSetup
   */
  virtual void initialSetup() override {}
  /**
   * This will read a the files again if they have been re-written from optimization iteration
   */
  virtual void timestepSetup() override;

protected:
  /// Mapping between adjoint simulation time and adjoint simulation time
  const Real & _reverse_time_end;
  /// The system time of the last instance the file was loaded
  std::time_t _file_mod_time;
  /// The forward simulation time for last instance the solution was updated
  Real _actual_interpolation_time;
};
