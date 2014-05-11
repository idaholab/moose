/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPLOTQUANTITY_H
#define RICHARDSPLOTQUANTITY_H

#include "GeneralPostprocessor.h"

class RichardsPlotQuantity;
class RichardsSumQuantity;

template<>
InputParameters validParams<RichardsPlotQuantity>();

/**
 * Extracts the value from RichardsSumQuantity userobject
 */
class RichardsPlotQuantity : public GeneralPostprocessor
{
public:
  RichardsPlotQuantity(const std::string & name, InputParameters parameters);
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
