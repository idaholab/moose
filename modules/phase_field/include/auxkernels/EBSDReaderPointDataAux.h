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
#include "EBSDReader.h"

// Forward Declarations

/**
 * This kernel makes data from the EBSDReader GeneralUserObject available
 * as AuxVariables.
 */
class EBSDReaderPointDataAux : public AuxKernel, EBSDAccessFunctors
{
public:
  static InputParameters validParams();

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
