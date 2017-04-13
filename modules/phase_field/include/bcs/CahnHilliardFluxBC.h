/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
