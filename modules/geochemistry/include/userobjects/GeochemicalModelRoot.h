//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MinimalGeochemicalSystem.h"

class GeochemicalModelRoot;

template <>
InputParameters validParams<GeochemicalModelRoot>();

/**
 * User object that parses a geochemical database file, and only retains information relevant to the
 * current geochemical model
 */
class GeochemicalModelRoot : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GeochemicalModelRoot(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /// provides a copy of the minimal geochemical database held by this object
  ModelGeochemicalDatabase getDatabase() const;

private:
  const MinimalGeochemicalSystem _model;
};
