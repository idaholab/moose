#ifndef NSENERGYBC_H
#define NSENERGYBC_H

#include "NSIntegratedBC.h"
#include "NSViscStressTensorDerivs.h"
#include "NSTemperatureDerivs.h"

// Forward Declarations
class NSEnergyBC;

template<>
InputParameters validParams<NSEnergyBC>();


/**
 * This class corresponds to the "natural" boundary condition
 * for the energy equation, i.e. what you get if you integrate
 * both the invsicid and viscous flux terms by parts:
 *
 * int_{Gamma} n . (rho*Hu - k*grad(T) - tau*u) v
 *
 * A typical use for this kernel would be a subsonic outflow BC in
 * which one physical value must be specified.  In this case, the
 * residual and Jacobian contrbutions of the k*grad(T) and
 * n.tau*u terms are computed and added to the matrix/rhs.  For the
 * enthalpy term, rho*H = rho*E + p, and the residual and Jacobian 
 * contributions are computed as normal except p is treated as given.
 */
class NSEnergyBC : public NSIntegratedBC
{

public:
  NSEnergyBC(const std::string & name, InputParameters parameters);

  virtual ~NSEnergyBC(){}

protected:
  
  /**
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled gradients
  VariableGradient& _grad_temperature;

  // Material properties
  MaterialProperty<Real>& _thermal_conductivity;

  // Parameters
  Real _cv;
  
  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself so we can access all of our data.
  NSViscStressTensorDerivs<NSEnergyBC> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;

  // A helper object for computing temperature gradient and Hessians.
  // Constructed via a reference to ourself so we can access all of our data.
  NSTemperatureDerivs<NSEnergyBC> _temp_derivs;

  // Declare ourselves a friend to the helper class
  template <class U>
  friend class NSTemperatureDerivs;

  // Single vector to refer to all gradients.  Initialized in
  // the ctor.
  std::vector<VariableGradient*> _gradU;
};


#endif // NSENERGYBC_H
