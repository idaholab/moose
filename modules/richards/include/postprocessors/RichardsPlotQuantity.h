/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSPLOTQUANTITY_H
#define RICHARDSPLOTQUANTITY_H

#include "GeneralPostprocessor.h"

class RichardsPlotQuantity;
class RichardsSumQuantity;

template <>
InputParameters validParams<RichardsPlotQuantity>();

/**
 * Extracts the value from RichardsSumQuantity userobject
 */
class RichardsPlotQuantity : public GeneralPostprocessor
{
public:
  RichardsPlotQuantity(const InputParameters & parameters);
  virtual ~RichardsPlotQuantity();

  virtual void initialize();
  virtual void execute();

  /// returns the value of the RichardsSumQuantity
  virtual PostprocessorValue getValue();

protected:
  /// the RichardsSumQuantity userobject
  const RichardsSumQuantity & _total_mass;
};

#endif /* RICHARDSPLOTQUANTITY_H */
