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
  /**
   * Get the forward problem time based on the adjoint backwards time stepping
   *
   * @return Real Forward simulation time
   */
  Real getActualTime() const { return _reverse_time_end ? *_reverse_time_end - _t + _dt : _t; }

private:
  const Real * const _reverse_time_end;
  std::time_t _file_mod_time;
};
