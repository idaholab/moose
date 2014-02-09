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
 * Just extracts the value from RichardsTotalOutflowMass
 */
class RichardsPlotQuantity : public GeneralPostprocessor
{
public:
  RichardsPlotQuantity(const std::string & name, InputParameters parameters);
  virtual ~RichardsPlotQuantity();

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();

protected:
  const RichardsSumQuantity & _total_mass;
};


#endif /* RICHARDSPLOTQUANTITY_H */
