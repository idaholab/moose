/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
