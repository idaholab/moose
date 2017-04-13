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

#ifndef MATERIALTESTINDICATOR_H
#define MATERIALTESTINDICATOR_H

// MOOSE includes
#include "Indicator.h"
#include "MaterialPropertyInterface.h"

// libMesh includes
#include "libmesh/quadrature.h"

// Forward Declarations
class MaterialTestIndicator;

template <>
InputParameters validParams<MaterialTestIndicator>();

/**
 * Computes the minimum element size.
 */
class MaterialTestIndicator : public Indicator
{
public:
  MaterialTestIndicator(const InputParameters & params);

protected:
  /// Computes the minimum element size based on the shear wave speed
  virtual void computeIndicator() override;

  /// Shear wave speed
  const MaterialProperty<Real> & _property;

  /// The current quadrature rule
  QBase *& _qrule;

  /// The variable for storing indicator value
  MooseVariable & _indicator_var;
};

#endif // MATERIALTESTINDICATOR_H
