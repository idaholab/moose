#ifndef NSSUPGMASS_H
#define NSSUPGMASS_H

#include "NSSUPGBase.h"

// Forward Declarations
class NSSUPGMass;

template<>
InputParameters validParams<NSSUPGMass>();

/**
 * Compute residual and Jacobian terms form the SUPG
 * terms in the mass equation.
 */
class NSSUPGMass : public NSSUPGBase
{
public:
  NSSUPGMass(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};

#endif // NSSUPGMASS_H
