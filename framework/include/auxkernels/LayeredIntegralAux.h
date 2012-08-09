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

#ifndef LAYEREDINTEGRALAUX_H
#define LAYEREDINTEGRALAUX_H

#include "AuxKernel.h"

//Forward Declarations
class LayeredIntegralAux;
class LayeredIntegral;

template<>
InputParameters validParams<LayeredIntegralAux>();

/**
 * Function auxiliary value
 */
class LayeredIntegralAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  LayeredIntegralAux(const std::string & name, InputParameters parameters);

  virtual ~LayeredIntegralAux() {}

protected:
  virtual Real computeValue();

  /// Function being used to compute the value of this kernel
  const LayeredIntegral & _layered_integral;
};

#endif // LAYEREDINTEGRALAUX_H
