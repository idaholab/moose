//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeReconstructionBase.h"

// Forward Declarations

/**
 * Multi-dimensional piecewise linear slope reconstruction
 * to get the slopes of cell average variables
 */
class SlopeReconstructionMultiD : public SlopeReconstructionBase
{
public:
  static InputParameters validParams();

  SlopeReconstructionMultiD(const InputParameters & parameters);

protected:
  /// store the pair of boundary ID and user object name
  std::map<BoundaryID, UserObjectName> _bnd_uo_name_map;
};
