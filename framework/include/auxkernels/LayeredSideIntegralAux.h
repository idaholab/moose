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

#ifndef LAYEREDSIDEINTEGRALAUX_H
#define LAYEREDSIDEINTEGRALAUX_H

#include "AuxKernel.h"

//Forward Declarations
class LayeredSideIntegralAux;
class LayeredSideIntegral;

template<>
InputParameters validParams<LayeredSideIntegralAux>();

/**
 * Function auxiliary value
 */
class LayeredSideIntegralAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  LayeredSideIntegralAux(const std::string & name, InputParameters parameters);

  virtual ~LayeredSideIntegralAux() {}

protected:
  virtual Real computeValue();

  /// Function being used to compute the value of this kernel
  const LayeredSideIntegral & _layered_integral;
};

#endif // LAYEREDSIDEINTEGRALAUX_H
