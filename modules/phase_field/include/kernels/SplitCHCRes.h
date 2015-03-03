/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHCRES_H
#define SPLITCHCRES_H

#include "SplitCHBase.h"


//Forward Declarations
class SplitCHCRes;

template<>
InputParameters validParams<SplitCHCRes>();

class SplitCHCRes : public SplitCHBase
{
public:
  SplitCHCRes(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  std::string _kappa_name;
  MaterialProperty<Real> & _kappa;
  unsigned int _w_var;
  VariableValue & _w;
};

#endif //SPLITCHCRES_H
