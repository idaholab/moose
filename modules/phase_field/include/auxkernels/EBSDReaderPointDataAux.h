/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EBSDREADERPOINTDATAAUX_H
#define EBSDREADERPOINTDATAAUX_H

#include "AuxKernel.h"
#include "EBSDReader.h"

// Forward Declarations
class EBSDReaderPointDataAux;

template <>
InputParameters validParams<EBSDReaderPointDataAux>();

/**
 * This kernel makes data from the EBSDReader GeneralUserObject available
 * as AuxVariables.
 */
class EBSDReaderPointDataAux : public AuxKernel, EBSDAccessFunctors
{
public:
  EBSDReaderPointDataAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  const EBSDReader & _ebsd_reader;

  /// MooseEnum that stores the type of data this AuxKernel extracts.
  MooseEnum _data_name;

  /// Accessor functor to fetch the selected data field form the EBSD data point
  MooseSharedPointer<EBSDPointDataFunctor> _val;

  /// precalculated element value
  Real _value;
};

#endif // EBSDREADERPOINTDATAAUX_H
