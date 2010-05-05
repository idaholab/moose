#ifndef SOLIDMECHTEMPCOUPLEY
#define SOLIDMECHTEMPCOUPLEY

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleY;

template<>
InputParameters validParams<SolidMechTempCoupleY>();


class SolidMechTempCoupleY : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleY(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};
 
#endif //SOLIDMECHTEMPCOUPLEY
