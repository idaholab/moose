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

  SolidMechZ(std::string name,
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
  
  unsigned int _y_var;
  std::vector<Real> & _y;
  std::vector<RealGradient> & _grad_y;
};
 
#endif //SOLIDMECHZ
