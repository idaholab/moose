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

#ifndef MATERIALMULTIPOINTSOURCE_H
#define MATERIALMULTIPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class MaterialMultiPointSource;

template <>
InputParameters validParams<MaterialMultiPointSource>();

/**
 * Similar to the ConstantPointSource, but evaluates a Material
 * property at the point source location instead of using a constant
 * value.
 */
class MaterialMultiPointSource : public DiracKernel
{
public:
  MaterialMultiPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  std::vector<Point> _points;

  const MaterialProperty<Real> & _value;
};

#endif // MATERIALMULTIPOINTSOURCE_H
