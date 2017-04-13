/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMTRACTIONFORM_H
#define INSMOMENTUMTRACTIONFORM_H

#include "INSMomentumBase.h"

// Forward Declarations
class INSMomentumTractionForm;

template <>
InputParameters validParams<INSMomentumTractionForm>();

/**
 * This class computes momentum equation residual and Jacobian viscous
 * contributions for the "traction" form of the governing equations.
 */
class INSMomentumTractionForm : public INSMomentumBase
{
public:
  INSMomentumTractionForm(const InputParameters & parameters);

  virtual ~INSMomentumTractionForm() {}

protected:
  virtual Real computeQpResidualViscousPart() override;
  virtual Real computeQpJacobianViscousPart() override;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) override;
};

#endif
