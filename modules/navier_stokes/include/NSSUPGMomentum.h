#ifndef NSSUPGMOMENTUM_H
#define NSSUPGMOMENTUM_H

#include "NSSUPGBase.h"

// Forward Declarations
class NSSUPGMomentum;

template<>
InputParameters validParams<NSSUPGMomentum>();

/**
 * Compute residual and Jacobian terms form the SUPG
 * terms in the momentum equation.
 */
class NSSUPGMomentum : public NSSUPGBase
{
public:
  NSSUPGMomentum(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  // This kernel is to be used for the x, y, and z momentum equations.
  // The _component parameter tells you which equation you are currently
  // solving.
  unsigned _component;
};

#endif // NSSUPGMOMENTUM_H
