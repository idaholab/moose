//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CAHNHILLIARDANISOFLUXBC_H
#define CAHNHILLIARDANISOFLUXBC_H

#include "CahnHilliardFluxBCBase.h"

class CahnHilliardAnisoFluxBC;

template <>
InputParameters validParams<CahnHilliardAnisoFluxBC>();

/**
 * Flux boundary condition for variable dependent anisotropic mobilities.
 */
class CahnHilliardAnisoFluxBC : public CahnHilliardFluxBCBase<RealTensorValue>
{
public:
  CahnHilliardAnisoFluxBC(const InputParameters & parameters);
};

#endif // CAHNHILLIARDANISOFLUXBC_H
