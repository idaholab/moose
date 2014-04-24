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

#ifndef INVERSEPOWERMETHOD_H
#define INVERSEPOWERMETHOD_H

#include "EigenExecutionerBase.h"

// Forward Declarations
class InversePowerMethod;

template<>
InputParameters validParams<InversePowerMethod>();

class InversePowerMethod : public EigenExecutionerBase
{
public:

  InversePowerMethod(const std::string & name, InputParameters parameters);

  virtual void execute();

protected:
  virtual void takeStep();

  const unsigned int & _min_iter;
  const unsigned int & _max_iter;
  const Real & _eig_check_tol;
  const Real & _pfactor;
  const bool & _cheb_on;
  bool _output_pi;
};

#endif //INVERSEPOWERMETHOD_H
