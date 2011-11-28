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

#ifndef ONOFFNEUMANNBC_H
#define ONOFFNEUMANNBC_H

#include "NeumannBC.h"

class OnOffNeumannBC;

template<>
InputParameters validParams<OnOffNeumannBC>();

/**
 * NeumanBC with ability to turn on and off
 */
class OnOffNeumannBC : public NeumannBC
{
public:
  OnOffNeumannBC(const std::string & name, InputParameters parameters);

  virtual bool shouldApply();

protected:

};

#endif /* ONOFFNEUMANNBC_H */
