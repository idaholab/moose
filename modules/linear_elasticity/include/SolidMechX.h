/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHX
#define SOLIDMECHX

#include "SolidMech.h"

//Forward Declarations
class SolidMechX;

template<>
InputParameters validParams<SolidMechX>();


class SolidMechX : public SolidMech
{
public:

  SolidMechX(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _mesh_dimension;

  unsigned int _y_var;
  VariableValue  & _y;
  VariableGradient & _grad_y;

  unsigned int _z_var;
  VariableValue  & _z;
  VariableGradient & _grad_z;
};
#endif //SOLIDMECHX
