//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "PerfGraphInterface.h"
#include "PostprocessorInterface.h"

class SolveObject;
class Executioner;
class FEProblemBase;
class DisplacedProblem;
class MooseMesh;
class SystemBase;
class AuxiliarySystem;

class SolveObject : public MooseObject, public PerfGraphInterface, public PostprocessorInterface
{
public:
  SolveObject(Executioner & ex);

  /**
   * Solve routine provided by this object.
   * @return True if solver is converged.
   */
  virtual bool solve() = 0;

  /// Set the inner solve object wrapped by this object.
  virtual void setInnerSolve(SolveObject & solve) { _inner_solve = &solve; }

protected:
  /// Executioner used to construct this
  Executioner & _executioner;
  /// Reference to FEProblem
  FEProblemBase & _problem;
  /// Displaced problem
  DisplacedProblem * _displaced_problem;
  /// Mesh
  MooseMesh & _mesh;
  /// Displaced mesh
  MooseMesh * _displaced_mesh;
  /// Reference to a system for creating vectors as needed for the solve, etc.
  SystemBase & _solver_sys;
  /// Reference to auxiliary system for faster access
  AuxiliarySystem & _aux;
  /// SolveObject wrapped by this solve object
  SolveObject * _inner_solve;
};
