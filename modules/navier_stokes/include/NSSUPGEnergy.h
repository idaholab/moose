#ifndef NSSUPGENERGY_H
#define NSSUPGENERGY_H

#include "NSSUPGBase.h"

// Forward Declarations
class NSSUPGEnergy;

template<>
InputParameters validParams<NSSUPGEnergy>();

/**
 * Compute residual and Jacobian terms form the SUPG
 * terms in the energy equation.
 */
class NSSUPGEnergy : public NSSUPGBase
{
public:
  NSSUPGEnergy(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};

#endif // NSSUPGENERGY_H
