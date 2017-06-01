/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#ifndef MATERIALLIMITTIMESTEP_H
#define MATERIALLIMITTIMESTEP_H

#include "ElementVariablePostprocessor.h"

class MaterialLimitTimeStep;

template<>
InputParameters validParams<MaterialLimitTimeStep>();

/**
 * This postporocessor calculates an estimated timestep size that limits
 * an auxiliary variable to below a given threshold.
*/
class MaterialLimitTimeStep : public ElementVariablePostprocessor
{
public:
  MaterialLimitTimeStep(const InputParameters & parameters);
  virtual void initialize();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  virtual void computeQpValue();

private:
  const Real _limit;
  const VariableValue & _u_old;
  Real _max_inc;
};

#endif // MATERIALLIMITTIMESTEP_H
