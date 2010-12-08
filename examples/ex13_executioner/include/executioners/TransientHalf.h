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

#ifndef TRANSIENTHALF_H
#define TRANSIENTHALF_H

#include "TransientExecutioner.h"

// Forward Declarations
class TransientHalf;

template<>
InputParameters validParams<TransientHalf>();

class TransientHalf: public TransientExecutioner
{
public:

  TransientHalf(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDT();

private:
  Real _ratio;
  Real _min_dt;
};

#endif //TRANSIENTHALF_H
