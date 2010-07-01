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

  SolidMechX(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _y_var;
  VariableValue  & _y;
  VariableGradient & _grad_y;

  unsigned int _z_var;
  VariableValue  & _z;
  VariableGradient & _grad_z;
};
#endif //SOLIDMECHX 
