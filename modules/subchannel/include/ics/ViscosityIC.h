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

#include "InitialCondition.h"

class SinglePhaseFluidProperties;

/**
 * Computes initial viscosity from specified pressure and temperature
 */
class ViscosityIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ViscosityIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Temperature
  const VariableValue & _T;
  /// Pressure
  const Real & _P;
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
