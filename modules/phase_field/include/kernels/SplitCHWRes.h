/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHWRES_H
#define SPLITCHWRES_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declaration
class SplitCHWRes;

template<>
InputParameters validParams<SplitCHWRes>();
/**
 * SplitCHWres creates the residual for the chemical
 * potential in the split form of the Cahn-Hilliard
 * equation.
 */
class SplitCHWRes : public DerivativeMaterialInterface<JvarMapInterface<Kernel> >
{
public:
  SplitCHWRes(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const std::string _mob_name;
  const MaterialProperty<Real> & _mob;

  std::vector<const MaterialProperty<Real> *> _dmobdarg;
};

#endif //SPLITCHWRES_H
