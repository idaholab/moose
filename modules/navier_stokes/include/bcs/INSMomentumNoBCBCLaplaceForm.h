/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMNOBCBCLAPLACEFORM_H
#define INSMOMENTUMNOBCBCLAPLACEFORM_H

#include "INSMomentumNoBCBCBase.h"

// Forward Declarations
class INSMomentumNoBCBCLaplaceForm;

template <>
InputParameters validParams<INSMomentumNoBCBCLaplaceForm>();

/**
 * This class implements the "No BC" boundary condition based on the
 * "Laplace" form of the viscous stress tensor.
 */
class INSMomentumNoBCBCLaplaceForm : public INSMomentumNoBCBCBase
{
public:
  INSMomentumNoBCBCLaplaceForm(const InputParameters & parameters);

  virtual ~INSMomentumNoBCBCLaplaceForm() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif
