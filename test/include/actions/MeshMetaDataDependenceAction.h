//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * This is a test class that is used to verify that Recoverable Mesh MetaData is available early
 * (e.g. around the time the MeshGenerators would normally be executed) so that Actions and other
 * objects can use that data.
 */
class MeshMetaDataDependenceAction : public Action
{
public:
  static InputParameters validParams();

  MeshMetaDataDependenceAction(const InputParameters & params);

  virtual void act();

private:
  const MeshGeneratorName & _generator_prefix;
};
