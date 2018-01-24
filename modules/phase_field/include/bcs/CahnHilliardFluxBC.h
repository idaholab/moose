//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CAHNHILLIARDFLUXBC_H
#define CAHNHILLIARDFLUXBC_H

#include "CahnHilliardFluxBCBase.h"

class CahnHilliardFluxBC;

template <>
InputParameters validParams<CahnHilliardFluxBC>();

/**
 * Flux boundary condition for variable dependent mobilities.
 */
class CahnHilliardFluxBC : public CahnHilliardFluxBCBase<Real>
{
public:
  CahnHilliardFluxBC(const InputParameters & parameters);
};

#endif // CAHNHILLIARDFLUXBC_H
