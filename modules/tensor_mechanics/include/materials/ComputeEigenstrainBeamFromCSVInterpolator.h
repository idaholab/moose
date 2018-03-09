//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEEIGENSTRAINBEAMFROMCSVINTERPOLATOR_H
#define COMPUTEEIGENSTRAINBEAMFROMCSVINTERPOLATOR_H

#include "ComputeEigenstrainBeamBase.h"

class ComputeEigenstrainBeamFromCSVInterpolator;
class CSVInterpolator;

template <>
InputParameters validParams<ComputeEigenstrainBeamFromCSVInterpolator>();

/**
 * ComputeEigenstrainBeamFromCSVInterpolator computes an Eigenstrain from csv files
 * containing displacement and rotational eigenstrains
 */
class ComputeEigenstrainBeamFromCSVInterpolator : public ComputeEigenstrainBeamBase
{
public:
  ComputeEigenstrainBeamFromCSVInterpolator(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// Component to which data from the csv files needs to be mapped to
  const unsigned int _to_component;

  /// Positon in coordinate directions other than _to_component to which data from csv files needs to be mapped to.
  const std::vector<Real> _position_vector;

  /// Vector storing the directions in which the position vector is defined.
  std::vector<Real> _other_components;

  /// Userobject that converts displacement eigenstrain csv files to bilinear interpolation objects
  const CSVInterpolator * const _disp_eigenstrain_uo;

  /// Userobject that converts rotation eigenstrain csv files to bilinear interpolation objects
  const CSVInterpolator * const _rot_eigenstrain_uo;

  /// Number of displacement eigenstrain variables in the csv files
  const unsigned int _ndisp;

  /// Number of rotational eigenstrain variables in the csv files
  const unsigned int _nrot;
};

#endif // COMPUTEEIGENSTRAINBEAMFROMCSVINTERPOLATOR_H
