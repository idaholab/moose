//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TransientMultiApp.h"
#include "BlockRestrictable.h"

class QPMultiApp;
class SubProblem;
class Assembly;

template <>
InputParameters validParams<QPMultiApp>();

/**
 * Automatically generates Sub-App positions from the quad points of elements in the master mesh.
 */
class QPMultiApp : public TransientMultiApp, public BlockRestrictable
{
public:
  static InputParameters validParams();

  QPMultiApp(const InputParameters & parameters);

protected:
  SubProblem & _subproblem;
  FEProblemBase & _fe_problem;
  THREAD_ID _tid;
  Assembly & _assembly;
  const QBase * const & _qrule;
  const MooseArray<Point> & _q_point;
  /**
   * fill in _positions with the positions of the sub-aps
   */
  virtual void fillPositions() override;
};
