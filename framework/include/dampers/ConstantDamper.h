/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONSTANTDAMPER_H
#define CONSTANTDAMPER_H

// Moose Includes
#include "Damper.h"

//Forward Declarations
class ConstantDamper;

template<>
InputParameters validParams<ConstantDamper>();

class ConstantDamper : public Damper
{
public:
  ConstantDamper(const std::string & name, InputParameters parameters);

protected:
  /**
   * This MUST be overriden by a child ConstantDamper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping();

  /**
   * The constant amount of the newton update to take.
   */
  Real _damping;
};
 
#endif //CONSTANTDAMPER_H
