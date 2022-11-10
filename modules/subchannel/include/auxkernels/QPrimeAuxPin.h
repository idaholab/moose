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

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate
 */
class QPrimeAuxPin : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  QPrimeAuxPin(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// rod diameter
  const Real & _rod_diameter;
};
