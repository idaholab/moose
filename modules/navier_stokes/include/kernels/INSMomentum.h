/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUM_H
#define INSMOMENTUM_H

#include "Kernel.h"

// Forward Declarations
class INSMomentum;

template<>
InputParameters validParams<INSMomentum>();

/**
 * This class computes momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentum : public Kernel
{
public:
  INSMomentum(const InputParameters & parameters);

  virtual ~INSMomentum(){}

  enum COORD_TYPE
  {
    XYZ,
    RZ,
    RSPHERICAL
  };

  static void setGeometryParameter(const InputParameters & params, COORD_TYPE & coord_type);

protected:
  virtual void computeResidual();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  bool _coord_type_set;
  COORD_TYPE _coord_type;

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _p;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_p;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  // Material properties
  // MaterialProperty<Real> & _dynamic_viscosity;
  Real _mu;
  Real _rho;
  RealVectorValue _gravity;

  // Parameters
  unsigned _component;
  bool _integrate_p_by_parts;
};


#endif // INSMOMENTUM_H
