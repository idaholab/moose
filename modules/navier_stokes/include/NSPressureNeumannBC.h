#ifndef NSPRESSURENEUMANNBC_H
#define NSPRESSURENEUMANNBC_H

#include "IntegratedBC.h"
//#include "Material.h"


// Forward Declarations
class NSPressureNeumannBC;

template<>
InputParameters validParams<NSPressureNeumannBC>();

/**
 * This kernel is appropriate for use with a "zero normal flow"
 * boundary condition in the context of the Euler equations.
 * In this situation, the convective term is integrated by parts
 * and the (rho*u)(u.n) term is zero since u.n=0.  Thus all we
 * are left with is the pressure times the normal.
 *
 * For the Navier-Stokes equations, a no-slip boundary condition
 * is probably what you want instead of this... for that use
 * NSImposedVelocityBC instead.
 */
class NSPressureNeumannBC : public IntegratedBC
{
public:

  NSPressureNeumannBC(const std::string & name, InputParameters parameters);

  virtual ~NSPressureNeumannBC(){}

protected:

  /**
   * This is here because materials don't yet work on boundaries!
   * Pressure is now computed as an Aux var
   */
  //Real pressure();
  
  virtual Real computeQpResidual();
  

  VariableValue & _pressure; // Aux Var
  
  int _component;

  MaterialProperty<Real> & _gamma; // Integrated BC, so can use Mat. properties
};

#endif //PRESSURENEUMANNBC_H
