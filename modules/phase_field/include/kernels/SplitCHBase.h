/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHBASE_H
#define SPLITCHBASE_H

#include "Kernel.h"


//Forward Declarations
class SplitCHBase;

template<>
InputParameters validParams<SplitCHBase>();

class SplitCHBase : public Kernel
{
public:
  SplitCHBase(const std::string & name, InputParameters parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeDEDC(PFFunctionType type);
};

#endif //SPLITCHBASE_H
