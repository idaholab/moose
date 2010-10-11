/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MAXINCREMENT_H
#define MAXINCREMENT_H

// Moose Includes
#include "Damper.h"

//Forward Declarations
class MaxIncrement;

template<>
InputParameters validParams<MaxIncrement>();

class MaxIncrement : public Damper
{
public:
  MaxIncrement(std::string name, MooseSystem & moose_system, InputParameters parameters);

protected:
  virtual Real computeQpDamping();

  /**
   * The maximum newton increment for the variable.
   */
  Real _max_increment;
};
 
#endif //MAXINCREMENT_H
