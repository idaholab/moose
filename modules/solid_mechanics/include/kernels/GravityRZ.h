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

#ifndef GRAVITYRZ_H
#define GRAVITYRZ_H

#include "Gravity.h"

//Forward Declarations
class GravityRZ;

template<>
InputParameters validParams<GravityRZ>();

class GravityRZ : public Gravity
{
public:

  GravityRZ(const std::string & name, InputParameters parameters);

  virtual ~GravityRZ() {}

protected:
  virtual Real computeQpResidual();

};

#endif
