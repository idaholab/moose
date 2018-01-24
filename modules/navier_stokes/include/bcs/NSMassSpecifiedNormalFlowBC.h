/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMASSSPECIFIEDNORMALFLOWBC_H
#define NSMASSSPECIFIEDNORMALFLOWBC_H

#include "NSMassBC.h"

// Forward Declarations
class NSMassSpecifiedNormalFlowBC;

template <>
InputParameters validParams<NSMassSpecifiedNormalFlowBC>();

/**
 * This class implements the mass equation boundary term with
 * a specified value of rho*(u.n) imposed weakly.
 *
 * Note: if you wish to impose rho*(u.n) = 0 weakly, you don't
 * actually need this class, that is the natural boundary condition.
 */
class NSMassSpecifiedNormalFlowBC : public NSMassBC
{
public:
  NSMassSpecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const Real _rhoun;
};

#endif // NSMASSSPECIFIEDNORMALFLOWBC_H
