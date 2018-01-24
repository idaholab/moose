/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EBSDREADERAVGDATAAUX_H
#define EBSDREADERAVGDATAAUX_H

#include "AuxKernel.h"
#include "EBSDAccessFunctors.h"

// Forward Declarations
class EBSDReaderAvgDataAux;
class EBSDReader;
class GrainTrackerInterface;

template <>
InputParameters validParams<EBSDReaderAvgDataAux>();

/**
 * This kernel makes data from the EBSDReader GeneralUserObject available
 * as AuxVariables.
 */
class EBSDReaderAvgDataAux : public AuxKernel, EBSDAccessFunctors
{
public:
  EBSDReaderAvgDataAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  const EBSDReader & _ebsd_reader;
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

#endif // EBSDREADERAVGDATAAUX_H
