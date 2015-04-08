/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHPARSED_H
#define CHPARSED_H

#include "CHBulk.h"
#include "DerivativeKernelInterface.h"
#include "JvarMapInterface.h"

//Forward Declarations
class CHParsed;

template<>
InputParameters validParams<CHParsed>();

/**
 * CHParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial.
 * \see SplitCHParsed
 */
class CHParsed : public DerivativeKernelInterface<JvarMapInterface<CHBulk> >
{
public:
  CHParsed(const std::string & name, InputParameters parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  std::vector<const MaterialProperty<Real>* > _second_derivatives;
  std::vector<const MaterialProperty<Real>* > _third_derivatives;
  std::vector<std::vector<const MaterialProperty<Real>* > > _third_cross_derivatives;
  std::vector<VariableGradient *> _grad_vars;
};

#endif // CHPARSED_H
