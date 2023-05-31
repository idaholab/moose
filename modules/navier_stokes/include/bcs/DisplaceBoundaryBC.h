/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#pragma once

#include "ADNodalBC.h"

class DisplaceBoundaryBC : public ADNodalBC
{
public:
  static InputParameters validParams();

  DisplaceBoundaryBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADReal & _velocity;
  const Real & _u_old;
};
