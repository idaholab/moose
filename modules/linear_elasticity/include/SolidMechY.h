#ifndef SOLIDMECHY
#define SOLIDMECHY

#include "SolidMech.h"


//Forward Declarations
class SolidMechY;

template<>
InputParameters valid_params<SolidMechY>();

class SolidMechY : public SolidMech
{
public:

  SolidMechY(std::string name,
             InputParameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _x_var;
  std::vector<Real> & _x;
  std::vector<RealGradient> & _grad_x;
  
  unsigned int _z_var;
  std::vector<Real> & _z;
  std::vector<RealGradient> & _grad_z;
};
#endif //SOLIDMECHY 
