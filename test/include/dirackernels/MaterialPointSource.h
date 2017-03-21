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

#ifndef MATERIALPOINTSOURCE_H
#define MATERIALPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class MaterialPointSource;

template <>
InputParameters validParams<MaterialPointSource>();

/**
 * Similar to the ConstantPointSource, but evaluates a Material
 * property at the point source location instead of using a constant
 * value.
 */
class MaterialPointSource : public DiracKernel
{
public:
  MaterialPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const Point & _p;

  const MaterialProperty<Real> * _value;
};

#endif // MATERIALPOINTSOURCE_H
