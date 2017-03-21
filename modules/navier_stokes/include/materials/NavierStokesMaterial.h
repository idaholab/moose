/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NAVIERSTOKESMATERIAL_H
#define NAVIERSTOKESMATERIAL_H

#include "Material.h"

// Forward Declarations
class NavierStokesMaterial;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NavierStokesMaterial>();

/**
 * This is the base class all materials should use if you are trying
 * to use the Navier-Stokes Kernels.
 *
 * Note that the derived class just needs to compute dynamic_viscocity
 * then call this class's computeProperties() function.
 *
 * Also make sure that the derived class's validParams function just
 * adds to this class's validParams.
 *
 * Finally, note that this Material _isn't_ registered with the
 * MaterialFactory.  The reason is that by itself this material
 * doesn't work!  You _must_ derive from this material and compute
 * dynamic_viscocity!
 */
class NavierStokesMaterial : public Material
{
public:
  NavierStokesMaterial(const InputParameters & parameters);

protected:
  /**
   * Must be called _after_ the child class computes dynamic_viscocity.
   */
  virtual void computeProperties();

  const unsigned int _mesh_dimension;

  const VariableGradient & _grad_u;
  const VariableGradient & _grad_v;
  const VariableGradient & _grad_w;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _dynamic_viscosity;

  // Also store the "cal A" matrices at each quadrature point as material
  // poperties.  These are defined in the compns notes and are needed for
  // computing strong and weak residual values and Jacobian entries, so it's
  // good if we reuse them...
  MaterialProperty<std::vector<RealTensorValue>> & _calA;

  // The "velocity column" matrices.  _calC[_qp][k] is a tensor with the
  // velocity vector in the k'th column.  See notes for additional details.
  MaterialProperty<std::vector<RealTensorValue>> & _calC;

  // The energy equation inviscid flux matrix components.  There are n_vars of
  // these for each dimension, so in 3D, 3*5=15 different matrices.
  // See notes for additional details.
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _calE;

  // Convenient storage for all of the velocity gradient components so
  // we can refer to them in a loop.
  std::vector<const VariableGradient *> _vel_grads;

  // Coupled values needed to compute strong form residuals
  // for SUPG stabilization...
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // Temperature is needed to compute speed of sound
  const VariableValue & _temperature;

  // Enthalpy is needed in computing energy equation strong residuals
  const VariableValue & _enthalpy;

  // Main solution variables are all needed for computing strong residuals
  const VariableValue & _rho;
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
  const VariableValue & _rho_E;

  // Time derivative values for dependent variables
  const VariableValue & _drho_dt;
  const VariableValue & _drhou_dt;
  const VariableValue & _drhov_dt;
  const VariableValue & _drhow_dt;
  const VariableValue & _drhoE_dt;

  // Gradients
  const VariableGradient & _grad_rho;
  const VariableGradient & _grad_rho_u;
  const VariableGradient & _grad_rho_v;
  const VariableGradient & _grad_rho_w;
  const VariableGradient & _grad_rho_E;

  // The real-valued material properties representing the element stabilization
  // parameters for each of the equations.
  MaterialProperty<Real> & _hsupg;
  MaterialProperty<Real> & _tauc;
  MaterialProperty<Real> & _taum;
  MaterialProperty<Real> & _taue;

  // The (vector-valued) material property which is the strong-form
  // residual at each quadrature point.
  MaterialProperty<std::vector<Real>> & _strong_residuals;

  // Fluid properties
  const IdealGasFluidProperties & _fp;

private:
  // To be called from computeProperties() function to compute _hsupg
  void computeHSUPG(unsigned int qp);

  // To be called from computeProperties() function to compute _tauc, _taum, _taue
  void computeTau(unsigned int qp);

  // To be called from computeProperties() function to compute the strong residual of each equation.
  void computeStrongResiduals(unsigned int qp);
};

#endif // NAVIERSTOKESMATERIAL_H
