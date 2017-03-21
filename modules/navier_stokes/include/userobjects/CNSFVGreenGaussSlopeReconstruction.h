/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVGREENGAUSSSLOPERECONSTRUCTION_H
#define CNSFVGREENGAUSSSLOPERECONSTRUCTION_H

#include "SlopeReconstructionMultiD.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVGreenGaussSlopeReconstruction;

template <>
InputParameters validParams<CNSFVGreenGaussSlopeReconstruction>();

/**
 * A user object that performs Green-Gauss slope reconstruction to get the slopes of the P0
 * primitive variables
 */
class CNSFVGreenGaussSlopeReconstruction : public SlopeReconstructionMultiD
{
public:
  CNSFVGreenGaussSlopeReconstruction(const InputParameters & parameters);

  /// compute the slope of the cell
  virtual void reconstructElementSlope();

protected:
  /// the input density
  MooseVariable * _rho;
  /// the input x-momentum
  MooseVariable * _rhou;
  /// the input y-momentum
  MooseVariable * _rhov;
  /// the input z-momentum
  MooseVariable * _rhow;
  /// the input total energy
  MooseVariable * _rhoe;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};

#endif
