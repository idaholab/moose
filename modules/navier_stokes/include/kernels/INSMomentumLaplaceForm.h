/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMLAPLACEFORM_H
#define INSMOMENTUMLAPLACEFORM_H

#include "INSMomentumBase.h"

// Forward Declarations
class INSMomentumLaplaceForm;

template <>
InputParameters validParams<INSMomentumLaplaceForm>();

/**
 * This class computes momentum equation residual and Jacobian viscous
 * contributions for the "Laplacian" form of the governing equations.
 */
class INSMomentumLaplaceForm : public INSMomentumBase
{
public:
  INSMomentumLaplaceForm(const InputParameters & parameters);

  virtual ~INSMomentumLaplaceForm() {}

protected:
  virtual Real computeQpResidualViscousPart() override;
  virtual Real computeQpJacobianViscousPart() override;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) override;
};

#endif
