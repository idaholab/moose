/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SLOPERECONSTRUCTIONMULTID_H
#define SLOPERECONSTRUCTIONMULTID_H

#include "SlopeReconstructionBase.h"

// Forward Declarations
class SlopeReconstructionMultiD;

template <>
InputParameters validParams<SlopeReconstructionMultiD>();

/**
 * Multi-dimensional piecewise linear slope reconstruction
 * to get the slopes of cell average variables
 */
class SlopeReconstructionMultiD : public SlopeReconstructionBase
{
public:
  SlopeReconstructionMultiD(const InputParameters & parameters);

protected:
  /// store the pair of boundary ID and user object name
  std::map<BoundaryID, UserObjectName> _bnd_uo_name_map;
};

#endif
