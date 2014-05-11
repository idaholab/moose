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
 * Sums into _total
 * This is used, for instance, to record the total mass
 * flowing into a borehole.
 * This is a suboptimal setup because it requires a const_cast
 * of a RichardsSumQuantity object in order to do the summing
 */
class RichardsSumQuantity : public GeneralUserObject
{
public:
  RichardsSumQuantity(const std::string & name, InputParameters parameters);
  virtual ~RichardsSumQuantity();

  /// sets _total = 0
  void zero();

  /**
   * adds contrib to _total
   * @param contrib the amount to add to _total
   */
  void add(Real contrib);

  /// does nothing
  virtual void initialize();

  /// does nothing
  virtual void execute();

  /// does MPI gather on _total
  virtual void finalize();

  /// returns _total
  virtual Real getValue() const;

protected:

  /// this holds the sum
  Real _total;
};

#endif /* RICHARDSSUMQUANTITY_H */
