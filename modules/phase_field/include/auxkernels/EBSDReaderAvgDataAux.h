//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "EBSDAccessFunctors.h"

// Forward Declarations
class EBSDReader;
class GrainTrackerInterface;

/**
 * This kernel makes data from the EBSDReader GeneralUserObject available
 * as AuxVariables.
 */
class EBSDReaderAvgDataAux : public AuxKernel, EBSDAccessFunctors
{
public:
  static InputParameters validParams();

  EBSDReaderAvgDataAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  /// Optional phase number needed for global grain index retrieval
  const unsigned int _phase;

  /// EBSD reader user object
  const EBSDReader & _ebsd_reader;

  /// Grain tracker user object
  const GrainTrackerInterface & _grain_tracker;

  /// MooseEnum that stores the type of data this AuxKernel extracts.
  MooseEnum _data_name;

  /// Accessor functor to fetch the selected data field form the EBSD data point
  MooseSharedPointer<EBSDAvgDataFunctor> _val;

  /// Value to return for points without active grains
  const Real _invalid;

  /// precalculated element value
  Real _value;
};
