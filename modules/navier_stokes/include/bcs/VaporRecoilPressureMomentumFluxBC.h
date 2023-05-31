/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#pragma once

#include "ADVectorIntegratedBC.h"

class VaporRecoilPressureMomentumFluxBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  VaporRecoilPressureMomentumFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _rc_pressure;
};
