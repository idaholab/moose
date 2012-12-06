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

#ifndef NODALL2NORM_H
#define NODALL2NORM_H

#include "NodalPostprocessor.h"

class MooseVariable;

//Forward Declarations
class NodalL2Norm;

template<>
InputParameters validParams<NodalL2Norm>();

class NodalL2Norm : public NodalPostprocessor
{
public:
  NodalL2Norm(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  Real _sum_of_squares;
};

#endif
