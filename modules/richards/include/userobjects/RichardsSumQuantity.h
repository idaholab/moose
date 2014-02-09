/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSUMQUANTITY_H
#define RICHARDSSUMQUANTITY_H

#include "GeneralUserObject.h"

class RichardsSumQuantity;

template<>
InputParameters validParams<RichardsSumQuantity>();

/**
 *
 */
class RichardsSumQuantity : public GeneralUserObject
{
public:
  RichardsSumQuantity(const std::string & name, InputParameters parameters);
  virtual ~RichardsSumQuantity();

  void zero();
  void add(Real contrib);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual Real getValue() const;

protected:
  Real _total_outflow_mass;
};

#endif /* RICHARDSSUMQUANTITY_H */
