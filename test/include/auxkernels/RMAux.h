//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GHOSTAUX_H
#define GHOSTAUX_H

#include "AuxKernel.h"

// Forward Declarations
class RMAux;
class AlgebraicRMTester;

template <>
InputParameters validParams<RMAux>();

/**
 * This AuxKernel retrieves values from the RMTest object and displays them in an elemental aux
 * field. This class throws an error if used on a Lagrange basis.
 */
class RMAux : public AuxKernel
{
public:
  RMAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  const AlgebraicRMTester & _rm_uo;
};

#endif // GHOSTAUX_H
