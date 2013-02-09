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

#ifndef SIDEFLUXAVERAGE_H
#define SIDEFLUXAVERAGE_H

#include "SideFluxIntegral.h"

//Forward Declarations
class SideFluxAverage;

template<>
InputParameters validParams<SideFluxAverage>();

class SideFluxAverage : public SideFluxIntegral
{
public:
  SideFluxAverage(const std::string & name, InputParameters parameters);
  virtual ~SideFluxAverage(){}

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  Real _volume;
};

#endif
