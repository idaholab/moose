/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAPHEATPOINTSOURCEMASTER_H
#define GAPHEATPOINTSOURCEMASTER_H

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

// Forward Declarations
class GapHeatPointSourceMaster;

template <>
InputParameters validParams<GapHeatPointSourceMaster>();

class GapHeatPointSourceMaster : public DiracKernel
{
public:
  GapHeatPointSourceMaster(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  PenetrationLocator & _penetration_locator;
  std::map<Point, PenetrationInfo *> point_to_info;
  NumericVector<Number> & _slave_flux;

  //  std::vector<Real> _localized_slave_flux;
};

#endif // GAPHEATPOINTSOURCEMASTER_H
