//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceUserObjectBase.h"

#include <set>

/**
 *  Base class for implementing interface user objects
 */
class InterfaceUserObject : public InterfaceUserObjectBase
{
public:
  static InputParameters validParams();

  InterfaceUserObject(const InputParameters & parameters);

protected:
  /**
   * Execute method.
   */
  virtual void execute() override;

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() override;

  /// The volume (or length) of the current neighbor
  const Real & getNeighborElemVolume();

  /// Whether finite volume variables are involved in the user object
  bool _has_fv_vars;

  /// A pointer to a face info, useful when working with FV
  const FaceInfo * _fi;

  /// A set of all the face infos that have been already looked at
  std::unordered_set<const FaceInfo *> _face_infos_processed;
};
