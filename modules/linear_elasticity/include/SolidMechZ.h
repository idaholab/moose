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

  SolidMechZ(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _x_var;
  MooseArray<Real> & _x;
  MooseArray<RealGradient> & _grad_x;
  
  unsigned int _y_var;
  MooseArray<Real> & _y;
  MooseArray<RealGradient> & _grad_y;
};
 
#endif //SOLIDMECHZ
