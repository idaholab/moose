/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHZ
#define SOLIDMECHZ

#include "SolidMech.h"

//Forward Declarations
class SolidMechZ;

template<>
InputParameters validParams<SolidMechZ>();

class SolidMechZ : public SolidMech
{
public:

  SolidMechZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _x_var;
  const VariableValue & _x;
  const VariableGradient & _grad_x;

  unsigned int _y_var;
  const VariableValue & _y;
  const VariableGradient & _grad_y;
};

#endif //SOLIDMECHZ
