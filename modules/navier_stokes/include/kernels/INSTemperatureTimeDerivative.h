#ifndef INSTEMPERATURETIMEDERIVATIVE_H
#define INSTEMPERATURETIMEDERIVATIVE_H

#include "TimeDerivative.h"

// Forward Declarations
class INSTemperatureTimeDerivative;

template<>
InputParameters validParams<INSTemperatureTimeDerivative>();

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
class INSTemperatureTimeDerivative : public TimeDerivative
{
public:
  INSTemperatureTimeDerivative(const std::string & name, InputParameters parameters);

  virtual ~INSTemperatureTimeDerivative(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Parameters
  Real _rho;
  Real _cp;
};


#endif // INSTEMPERATURETIMEDERIVATIVE_H
