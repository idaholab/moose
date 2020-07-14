#pragma once

#include "ADMaterial.h"
#include "DerivativeMaterialInterface.h"

class NavierStokesSUPGMaterial;

class SinglePhaseFluidProperties;
class IdealGasFluidProperties;

declareADValidParams(NavierStokesSUPGMaterial);

/**
 * Material providing the coupled equation Streamline Upwind Petrov Galerkin
 * (SUPG) stabilization for the fluid energy equation. A detailed description
 * of the stabilization is provided in the theory manual.
 */
class NavierStokesSUPGMaterial : public DerivativeMaterialInterface<ADMaterial>
{
public:
  NavierStokesSUPGMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  // functions for computing derivatives
  void computeIdealGasdPdT();
  void computeSinglePhasedPdT();

  /// Compute partial derivatives of enthalpy w.r.t. conservation variables
  void computeEnthalpyDerivatives();

  // functions for computing some of the materials (to simplify source code)
  void computeA();
  void computeF();
  void computeK();
  void computeG();
  void computeS();
  void computedUdt();
  void computedUdx();
  void computeR();

  // functions for printing results for debugging
  void printMatrices();
  void printState();

  /// dimension of the mesh
  const unsigned int _mesh_dim;

  /// number of coupled equations for the fluid
  const unsigned int _N;

  // integer representing the RZ-coordinate axis
  const unsigned int _rz_coord;

  /// Whether the conservation of mass equation should be included. This is primarily
  /// used for testing kernels not present in the mass equation to simplify input
  /// file structure.
  const bool & _mass_eqn;

  /// Whether the conservation of momentum equation(s) should be included. This is
  /// primarily used for testing kernels not present in the momentum equation(s) to
  /// simplify input file structure.
  const bool & _momentum_eqn;

  /// Whether the conservation of fluid energy equation should be included. This is
  /// primarily used for testing kernels not present in the fluid energy equation to
  /// simplify input file structure.
  const bool & _fluid_energy_eqn;

  /// Whether to include the viscous stress term in the momentum equation; this should
  /// be set to 'true' if the 'MomentumDiffusiveFlux' kernel is used in the momentum equations
  const bool _viscous_stress;

  // nonlinear variables
  const ADMaterialProperty<Real> & _rho;
  const ADMaterialProperty<RealVectorValue> & _momentum;
  const ADMaterialProperty<Real> & _rho_et;

  // nonlinear variables - gradients
  const ADMaterialProperty<RealVectorValue> & _grad_rho;
  const ADMaterialProperty<RealVectorValue> & _grad_rho_u;
  const ADMaterialProperty<RealVectorValue> & _grad_rho_v;
  const ADMaterialProperty<RealVectorValue> & _grad_rho_w;
  const ADMaterialProperty<RealVectorValue> & _grad_rho_et;

  /// Time derivative of fluid density, only required if the problem is transient
  /// and the mass equation is included
  const ADMaterialProperty<Real> & _drho_dt;

  /// Time derivative of the x-component of momentum, only required if the problem is
  /// transient and the momentum equation(s) are included
  const ADMaterialProperty<Real> & _drhou_dt;

  /// Time derivative of the y-component of momentum, only required if the problem is
  /// transient, the momentum equation(s) are included, and this variable is provided
  const ADMaterialProperty<Real> & _drhov_dt;

  /// Time derivative of the z-component of momentum, only required if the problem is
  /// transient, the momentum equation(s) are included, and this variable is provided
  const ADMaterialProperty<Real> & _drhow_dt;

  /// Time derivative of the total fluid energy per unit mass, only required if the
  /// problem is transient and the fluid energy equation is included
  const ADMaterialProperty<Real> & _drhoEt_dt;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// x-component of velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_x;

  /// y-component of velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_y;

  /// z-component of velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_z;

  /// x-component of velocity second gradient
  const ADMaterialProperty<RealTensorValue> & _grad_grad_vel_x;

  /// y-component of velocity second gradient
  const ADMaterialProperty<RealTensorValue> & _grad_grad_vel_y;

  /// z-component of velocity second gradient
  const ADMaterialProperty<RealTensorValue> & _grad_grad_vel_z;

  /// Porosity
  const VariableValue & _eps;

  /// gradient of porosity
  const VariableGradient & _grad_eps;

  /// pressure
  const ADMaterialProperty<Real> & _p;

  /// enthalpy
  const ADMaterialProperty<Real> & _enthalpy;

  /// specific internal energy
  const ADMaterialProperty<Real> & _e;

  /// specific volume
  const ADMaterialProperty<Real> & _v;

  /// speed
  const ADMaterialProperty<Real> & _speed;

  /// Fluid properties object, only required if the momentum or energy equations
  /// are included
  const SinglePhaseFluidProperties * _fluid;

  /// Ideal gas fluid properties object, which permits simpler evaluations of $A$
  const IdealGasFluidProperties * _fluid_ideal_gas;

  // for computing source terms
  const RealVectorValue & _acceleration;

  /// Fluid effective thermal conductivity
  const ADMaterialProperty<Real> * _kappa;

  /// Derivative of fluid effective thermal conductivity with respect to pressure
  const MaterialProperty<Real> * _dkappa_dp;

  /// Derivative of fluid effective thermal conductivity with respect to fluid temperature
  const MaterialProperty<Real> * _dkappa_dT;

  /// Fluid effective viscosity
  const ADMaterialProperty<Real> * _mu_eff;

  /// Derivative of fluid effective viscosity with respect to pressure
  const MaterialProperty<Real> * _dmu_eff_dp;

  /// Derivative of fluid effective viscosity with respect to fluid temperature
  const MaterialProperty<Real> * _dmu_eff_dT;

  /// Whether the convective heat transfer kernel exists
  const bool _convective_heat_transfer;

  /// Convective heat transfer coefficient
  const ADMaterialProperty<Real> * _alpha;

  /// Solid temperature
  const ADVariableValue & _T_solid;

  /// Linear drag coefficient, only required if the momentum equation is included
  const ADMaterialProperty<RealVectorValue> * _cL;

  /// Quadratic drag coefficient, only required if the momentum equation is included
  const ADMaterialProperty<RealVectorValue> * _cQ;

  /// Fluid temperature
  const ADMaterialProperty<Real> & _T_fluid;

  /// Spatial gradient of fluid temperature
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid;

  /// Second spatial gradient of fluid temperature
  const ADMaterialProperty<RealTensorValue> & _hess_T_fluid;

  /// Spatial gradient of pressure
  const ADMaterialProperty<RealVectorValue> & _grad_pressure;

  /// Superficial heat source in the fluid phase, only required if the fluid energy
  /// equation is included
  const VariableValue & _heat_source;

  /// Scaling factor applied to the heat source
  const Real & _scaling_factor;

  // inviscid flux Jacobian matrix
  ADMaterialProperty<std::vector<DenseMatrix<Real>>> & _A;

  // R(U), the strong residuals
  ADMaterialProperty<DenseVector<Real>> & _R;

  // S(U), the vector of source terms
  ADMaterialProperty<DenseVector<Real>> & _S;

  // F(U), the inviscid flux vectors
  ADMaterialProperty<std::vector<DenseVector<Real>>> & _F;

  // G(U), the viscous flux vectors
  ADMaterialProperty<std::vector<DenseVector<Real>>> & _G;

  /// $\frac{\partial\vec{U}}{\partial t}$, the vector of conserved variable
  /// time derivatives. There is a single vector of these time derivatives.
  ADMaterialProperty<DenseVector<Real>> & _dU_dt;

  void (NavierStokesSUPGMaterial::*_single_phase_dP_and_dT)();

  // Variables which are used by the _single_phase_dP_and_dT() function,
  // and must be initialized at each qp.
  ADReal _vel2;
  ADReal _total_energy;

  // terms in arrays for loop access
  // std::vector<Real> _U;
  std::vector<ADReal> _dP, _dh, _dT;

  /// Vectors of conserved variable gradient components in each spatial dimension.
  /// The first index corresponds to the spatial dimension, while the second index
  /// corresponds to the equation number.
  std::vector<DenseVector<ADReal>> _dU_dx;

  // Helper function that prints a matrix in a format that can be read by Numpy.
  // TODO: This could possibly be added in libMesh as a function on DenseMatrix.
  void print_numpy(const DenseMatrix<Real> & mat) const;
};
