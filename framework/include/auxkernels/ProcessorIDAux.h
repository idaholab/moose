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

#ifndef PROCESSORIDAUX_H
#define PROCESSORIDAUX_H

#include "AuxKernel.h"


//Forward Declarations
class ProcessorIDAux;

template<>
InputParameters validParams<ProcessorIDAux>();

/**
 * ProcessorID auxiliary value (can be used only as an elemental kernel)
 *
 */
class ProcessorIDAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ProcessorIDAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();
};

#endif //PROCESSORIDAUX_H
