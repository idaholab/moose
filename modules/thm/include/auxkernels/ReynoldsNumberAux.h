#ifndef REYNOLDSNUMBERAUX_H
#define REYNOLDSNUMBERAUX_H

#include "AuxKernel.h"

class ReynoldsNumberAux;
class IAPWS95FluidProperties;

template<>
InputParameters validParams<ReynoldsNumberAux>();

/**
 * Computes Reynolds number
 */
class ReynoldsNumberAux : public AuxKernel
{
public:
  ReynoldsNumberAux(const InputParameters & parameters);
  virtual ~ReynoldsNumberAux();

protected:
  virtual Real computeValue();

  /// Volume fraction
  const VariableValue & _alpha;
  /// Density of the phase
  const VariableValue & _rho;
  /// Velocity of the phase
  const VariableValue & _u_vel;
  /// Hydraulic diameter
  const VariableValue & _Dh;
  /// Specific volume
  const VariableValue & _v;
  /// Specific internal energy
  const VariableValue & _e;

  const IAPWS95FluidProperties & _fp;
};

#endif /* REYNOLDSNUMBERAUX_H */
