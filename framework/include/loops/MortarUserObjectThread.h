//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MortarExecutorInterface.h"

#include "libmesh/libmesh_common.h"

class MortarUserObject;
class SubProblem;
class FEProblemBase;
class AutomaticMortarGeneration;
class Assembly;

class MortarUserObjectThread : public MortarExecutorInterface
{
public:
  MortarUserObjectThread(std::vector<MortarUserObject *> & mortar_user_objects,
                         const AutomaticMortarGeneration & amg,
                         SubProblem & subproblem,
                         FEProblemBase & fe_problem,
                         bool displaced,
                         Assembly & assembly);

  /**
   * Loops over the mortar segment mesh and executes the user objects
   */
  void operator()();

private:
  /// The mortar user objects to loop over when on each mortar segment element
  std::vector<MortarUserObject *> & _mortar_user_objects;

  /// Automatic mortar generation (amg) object providing the mortar mesh to loop over
  const AutomaticMortarGeneration & _amg;

  /// A reference to the SubProblem object for reiniting lower-dimensional element quantities
  SubProblem & _subproblem;

  /// A reference to the FEProblemBase object for reiniting higher-dimensional element and neighbor
  /// element quantities. We use the FEProblemBase object for reiniting these because we may be
  /// using material properties from either undisplaced or displaced materials
  FEProblemBase & _fe_problem;

  /// Whether the mortar user objects are operating on the displaced mesh
  const bool _displaced;

  /// A reference to the assembly object
  Assembly & _assembly;
};
