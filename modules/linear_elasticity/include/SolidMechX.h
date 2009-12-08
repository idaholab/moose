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

  SolidMechX(std::string name,
             InputParameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _y_var;
  std::vector<Real> & _y;
  std::vector<RealGradient> & _grad_y;

  unsigned int _z_var;
  std::vector<Real> & _z;
  std::vector<RealGradient> & _grad_z;
};
#endif //SOLIDMECHX 
