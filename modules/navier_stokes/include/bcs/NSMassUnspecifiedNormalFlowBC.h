/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMASSUNSPECIFIEDNORMALFLOWBC_H
#define NSMASSUNSPECIFIEDNORMALFLOWBC_H

#include "NSMassBC.h"

// Forward Declarations
class NSMassUnspecifiedNormalFlowBC;

template <>
InputParameters validParams<NSMassUnspecifiedNormalFlowBC>();

/**
 * This class implements the mass equation boundary term with
 * the rho*(u.n) boundary integral computed implicitly.
 */
class NSMassUnspecifiedNormalFlowBC : public NSMassBC
{
public:
  NSMassUnspecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSMASSUNSPECIFIEDNORMALFLOWBC_H
