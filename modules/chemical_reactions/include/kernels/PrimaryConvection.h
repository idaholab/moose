/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Kernel.h"

#ifndef PRIMARYCONVECTION_H
#define PRIMARYCONVECTION_H

// Forward Declaration
class PrimaryConvection;

template<>
InputParameters validParams<PrimaryConvection>();

/**
 * Define the Kernel for a PrimaryConvection operator that looks like:
 *
 * cond * grad_pressure * grad_u
 *
 * This first line is defining the name and inheriting from Kernel.
 */
class PrimaryConvection : public Kernel
{
public:

  /**
   * This is the Constructor declaration AND definition.
   * It is ok to have the definition in the .h if the function body
   * is really small.  Otherwise it should be in the .C
   */
  PrimaryConvection(const InputParameters & parameters);

protected:
  /**
   * Responsible for computing the residual at one quadrature point
   *
   * This should always be defined in the .C
   * @return The residual of the convection of primary species.
   */
  virtual Real computeQpResidual();

  /**
   * Responsible for computing the diagonal block of the preconditioning matrix.
   * This is essentially the partial derivative of the residual with respect to
   * the variable this kernel operates on ("u").
   *
   * Note that this can be an approximation or linearization.  In this case it's
   * not because the Jacobian of this operator is easy to calculate.
   *
   * This should always be defined in the .C
   * @return The diagonal jacobian of the convection of primary species.
   */
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Material property of hydraulic conductivity
  const MaterialProperty<Real> & _cond;

private:
  /**
   * Coupled things come through as std::vector _refernces_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
  /// Coupled gradient of hydraulic head.
  const VariableGradient & _grad_p;
};

#endif //PRIMARYCONVECTION_H
