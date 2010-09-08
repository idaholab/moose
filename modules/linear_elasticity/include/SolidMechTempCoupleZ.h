#ifndef SOLIDMECHTEMPCOUPLEZ
#define SOLIDMECHTEMPCOUPLEZ

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleZ;

template<>
InputParameters validParams<SolidMechTempCoupleZ>();


class SolidMechTempCoupleZ : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleZ(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};
 
#endif //SOLIDMECHTEMPCOUPLEZ
