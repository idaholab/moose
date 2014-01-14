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

#ifndef POSTPROCESSORDT_H
#define POSTPROCESSORDT_H

#include "TimeStepper.h"
#include "PostprocessorInterface.h"

class PostprocessorDT;

template<>
InputParameters validParams<PostprocessorDT>();

/**
 * Computes the value of dt based on a postprocessor value
 */
class PostprocessorDT :
    public TimeStepper,
    public PostprocessorInterface
{
public:
  PostprocessorDT(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeInitialDT();
  virtual Real computeDT();

  PostprocessorValue & _pps_value;
  bool _has_initial_dt;
  Real _initial_dt;
};


#endif /* POSTPROCESSORDT_H */
