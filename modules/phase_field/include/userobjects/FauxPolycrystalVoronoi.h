//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalVoronoi.h"

// Forward Declarations
class FauxPolycrystalVoronoi;

class FauxPolycrystalVoronoi : public PolycrystalVoronoi
{
public:
  static InputParameters validParams();

  FauxPolycrystalVoronoi(const InputParameters & parameters);

  /**
   * We override all these functions to avoid calling FeatureFloodCount
   * We know here is a one-to-one mapping between grain and variable
   */
  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;
  virtual void meshChanged() override {}
};
