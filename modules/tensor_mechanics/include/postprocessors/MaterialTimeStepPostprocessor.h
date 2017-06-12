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

#ifndef MATERIALTIMESTEPPOSTPROCESSOR_H
#define MATERIALTIMESTEPPOSTPROCESSOR_H

#include "ElementPostprocessor.h"

class MaterialTimeStepPostprocessor;

template <>
InputParameters validParams<MaterialTimeStepPostprocessor>();

/**
 * This postporocessor calculates an estimated timestep size that limits
 * an auxiliary variable to below a given threshold.
*/
class MaterialTimeStepPostprocessor : public ElementPostprocessor
{
public:
  MaterialTimeStepPostprocessor(const InputParameters & parameters);
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  const MaterialProperty<Real> & _matl_time_step;

  Real _value;
  unsigned int _qp;
};

#endif // MATERIALTIMESTEPPOSTPROCESSOR_H
