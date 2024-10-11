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

#include "AuxKernel.h"

/**
 * Computes mass float rate from specified uniform mass flux and cross-sectional area.
 * Reads mass flux value from postprocessor.
 */
class MassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Mass flux provided by postprocessor
  const PostprocessorValue & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
};
