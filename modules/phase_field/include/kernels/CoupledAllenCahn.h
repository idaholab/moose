/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDALLENCAHN_H
#define COUPLEDALLENCAHN_H

#include "ACBulk.h"

// Forward Declarations
class CoupledAllenCahn;

template <>
InputParameters validParams<CoupledAllenCahn>();

/**
 * CoupledAllenCahn uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to compute the residual
 * for the bulk part of the Allen-Cahn equation, where the variational
 * free energy derivative is taken w.r.t. a coupled variable.
 */
class CoupledAllenCahn : public ACBulk<Real>
{
public:
  CoupledAllenCahn(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // coupled variable name
  VariableName _v_name;

  const unsigned int _nvar;
  const MaterialProperty<Real> & _dFdV;
  const MaterialProperty<Real> & _d2FdVdEta;

  std::vector<const MaterialProperty<Real> *> _d2FdVdarg;
};

#endif // COUPLEDALLENCAHN_H
