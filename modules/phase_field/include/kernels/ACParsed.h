/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACPARSED_H
#define ACPARSED_H

#include "ACBulk.h"
#include "JvarMapInterface.h"
#include "DerivativeKernelInterface.h"

//Forward Declarations
class ACParsed;

template<>
InputParameters validParams<ACParsed>();

/**
 * ACParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to computer the
 * residual for the bulk part of the Allen-Cahn equation.
 */
class ACParsed : public DerivativeKernelInterface<JvarMapInterface<ACBulk> >
{
public:
  ACParsed(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _dFdEta;
  const MaterialProperty<Real> & _d2FdEta2;

  std::vector<const MaterialProperty<Real> *> _d2FdEtadarg;
};

#endif // ACPARSED_H
