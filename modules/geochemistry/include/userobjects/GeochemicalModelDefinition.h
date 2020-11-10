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
#include "PertinentGeochemicalSystem.h"

/**
 * User object that parses a geochemical database file, and only retains information relevant to the
 * current geochemical model
 */
class GeochemicalModelDefinition : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GeochemicalModelDefinition(const InputParameters & parameters);

  virtual void initialize() override final;
  virtual void execute() override final;
  virtual void finalize() override final;

  /// provides a reference to the pertinent geochemical database held by this object
  const ModelGeochemicalDatabase & getDatabase() const;

  /// provides a reference to the PertinentGeochemicalSystem of this object
  const PertinentGeochemicalSystem & getPertinentGeochemicalSystem() const;

  /// provides a reference to the full database file used by this object
  const GeochemicalDatabaseReader & getOriginalFullDatabase() const;

private:
  const GeochemicalDatabaseReader _db;
  PertinentGeochemicalSystem _model;
};
