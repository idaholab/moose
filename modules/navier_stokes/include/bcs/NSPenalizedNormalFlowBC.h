/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSPENALIZEDNORMALFLOWBC_H
#define NSPENALIZEDNORMALFLOWBC_H

#include "NSIntegratedBC.h"

// Forward Declarations
class NSPenalizedNormalFlowBC;

template <>
InputParameters validParams<NSPenalizedNormalFlowBC>();

/**
 * This class penalizes the the value of u.n on the boundary
 * so that it matches some desired value.
 */
class NSPenalizedNormalFlowBC : public NSIntegratedBC
{
public:
  NSPenalizedNormalFlowBC(const InputParameters & parameters);

protected:
  /**
   * The standard interface functions.
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const Real _penalty;
  const Real _specified_udotn;
};

#endif // NSPENALIZEDNORMALFLOWBC_H
