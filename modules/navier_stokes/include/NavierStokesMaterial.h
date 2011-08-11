#ifndef NAVIERSTOKESMATERIAL_H
#define NAVIERSTOKESMATERIAL_H

#include "Material.h"


//Forward Declarations
class NavierStokesMaterial;

template<>
InputParameters validParams<NavierStokesMaterial>();

/**
 * This is the base class all materials should use if you are trying to use the Navier-Stokes Kernels.
 *
 * Note that the derived class just needs to compute dynamic_viscocity then call this class's
 * computeProperties() function.
 *
 * Also make sure that the derived class's validParams function just adds to this class's validParams.
 *
 * Finally, note that this Material _isn't_ registered with the MaterialFactory.  The reason is that by
 * itself this material doesn't work!  You _must_ derive from this material and compute dynamic_viscocity!
 */
class NavierStokesMaterial : public Material
{
public:
  NavierStokesMaterial(const std::string & name, InputParameters parameters);
  
protected:

  /**
   * Must be called _after_ the child class computes dynamic_viscocity.
   */
  virtual void computeProperties();

  VariableGradient & _grad_u;
  VariableGradient & _grad_v;
  VariableGradient & _grad_w;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _dynamic_viscosity;

  std::vector<VariableGradient *> _vel_grads;

  // Specific heat at constant volume, treated as a single
  // constant values.
  Real _R;
  Real _gamma;
  Real _Pr;
  Real _cv;

  // Coupled values needed to compute strong form residuals
  // for SUPG stabilization...
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  
  // Temperature is needed to compute speed of sound
  VariableValue & _temperature;
  
  // Enthalpy is needed in computing energy equation strong residuals
  VariableValue& _enthalpy;
  
  // Main solution variables are all needed for computing strong residuals
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e;

  // Also need "old" (from previous timestep) coupled variable values
  // for approximating time derivatives in strong residuals.
  VariableValue& _rho_old;
  VariableValue& _rho_u_old;
  VariableValue& _rho_v_old;
  VariableValue& _rho_w_old;
  VariableValue& _rho_e_old;

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;

  // The real-valued material properties representing the element stabilization
  // parameters for each of the equations.
  MaterialProperty<Real> & _hsupg;
  MaterialProperty<Real> & _tauc;
  MaterialProperty<Real> & _taum;
  MaterialProperty<Real> & _taue;
  
  // The (vector-valued) material property which is the strong-form
  // residual at each quadrature point.
  // FIXME: Can we resize this vector directly in computeProperties?
  MaterialProperty<std::vector<Real> > & _strong_residuals;

private:
  // To be called from computeProperties() function to compute _hsupg
  void compute_h_supg(unsigned qp);

  // To be called from computeProperties() function to compute _tauc, _taum, _taue
  void compute_tau(unsigned qp);

  // To be called from computeProperties() function to compute the strong residual of each equation.
  void compute_strong_residuals(unsigned qp);

  // Reference to a pointer to an FEBase object.  Initialized in ctor.
  FEBase*& _fe;

  // Constant references to finite element mapping data
  const std::vector<Real>& _dxidx;
  const std::vector<Real>& _dxidy;
  const std::vector<Real>& _dxidz;

  const std::vector<Real>& _detadx;
  const std::vector<Real>& _detady;
  const std::vector<Real>& _detadz;

  // In 2D, these vectors will be empty...
  const std::vector<Real>& _dzetadx;
  const std::vector<Real>& _dzetady;
  const std::vector<Real>& _dzetadz;

};

#endif //NAVIERSTOKESMATERIAL_H
