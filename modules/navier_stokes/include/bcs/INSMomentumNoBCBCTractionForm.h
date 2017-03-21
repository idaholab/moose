/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMNOBCBCTRACTIONFORM_H
#define INSMOMENTUMNOBCBCTRACTIONFORM_H

#include "INSMomentumNoBCBCBase.h"

// Forward Declarations
class INSMomentumNoBCBCTractionForm;

template <>
InputParameters validParams<INSMomentumNoBCBCTractionForm>();

/**
 * This class implements the "No BC" boundary condition based on the
 * "traction" form of the viscous stress tensor.
 */
class INSMomentumNoBCBCTractionForm : public INSMomentumNoBCBCBase
{
public:
  INSMomentumNoBCBCTractionForm(const InputParameters & parameters);

  virtual ~INSMomentumNoBCBCTractionForm() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif
