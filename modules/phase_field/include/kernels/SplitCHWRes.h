/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHWRES_H
#define SPLITCHWRES_H

#include "Kernel.h"


//Forward Declarations
class SplitCHWRes;

template<>
InputParameters validParams<SplitCHWRes>();

class SplitCHWRes : public Kernel
{
public:
  SplitCHWRes(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  std::string _mob_name;
  MaterialProperty<Real> & _mob;
};

#endif //SPLITCHWRES_H
