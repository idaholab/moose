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

#ifndef STATEFULPOINTSOURCE_H
#define STATEFULPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class StatefulPointSource;

template<>
InputParameters validParams<StatefulPointSource>();

/**
 * Similar to the MaterialPointSource, but uses getMaterialPropertyOld
 * to induce the stateful material property machinery, which is not
 * supported for DiracKernels.
 */
class StatefulPointSource : public DiracKernel
{
public:
  StatefulPointSource(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  Point _p;

  MaterialProperty<Real> & _value;
};

#endif //STATEFULPOINTSOURCE_H
