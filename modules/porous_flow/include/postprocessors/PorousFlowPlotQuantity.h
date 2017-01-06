/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWPLOTQUANTITY_H
#define POROUSFLOWPLOTQUANTITY_H

#include "GeneralPostprocessor.h"

class PorousFlowPlotQuantity;
class PorousFlowSumQuantity;

template<>
InputParameters validParams<PorousFlowPlotQuantity>();

/**
 * Extracts the value from PorousFlowSumQuantity userobject
 */
class PorousFlowPlotQuantity : public GeneralPostprocessor
{
public:
  PorousFlowPlotQuantity(const InputParameters & parameters);
  virtual ~PorousFlowPlotQuantity();

  virtual void initialize() override;
  virtual void execute() override;

  /// returns the value of the PorousFlowSumQuantity
  virtual PostprocessorValue getValue() override;

protected:
  /// the PorousFlowSumQuantity userobject
  const PorousFlowSumQuantity & _total_mass;
};


#endif /* POROUSFLOWPLOTQUANTITY_H */
