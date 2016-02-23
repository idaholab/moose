/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHY
#define SOLIDMECHY

#include "SolidMech.h"

//Forward Declarations
class SolidMechY;

template<>
InputParameters validParams<SolidMechY>();


class SolidMechY : public SolidMech
{
public:

  SolidMechY(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _mesh_dimension;

  unsigned int _x_var;
  const VariableValue & _x;
  const VariableGradient & _grad_x;

  unsigned int _z_var;
  const VariableValue & _z;
  const VariableGradient & _grad_z;
};
#endif //SOLIDMECHY
