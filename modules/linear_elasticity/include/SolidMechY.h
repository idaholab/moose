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

  SolidMechY(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _x_var;
  MooseArray<Real> & _x;
  MooseArray<RealGradient> & _grad_x;
  
  unsigned int _z_var;
  MooseArray<Real> & _z;
  MooseArray<RealGradient> & _grad_z;
};
#endif //SOLIDMECHY 
