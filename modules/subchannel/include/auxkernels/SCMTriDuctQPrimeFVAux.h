/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "DiffusionFluxFVAux.h"

/**
 * Computes linear heat rate
 */
class SCMTriDuctQPrimeFVAux : public DiffusionFluxFVAux
{
public:
  static InputParameters validParams();

  SCMTriDuctQPrimeFVAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// flat-to-flat distance
  const Real & _flat_to_flat;
};
