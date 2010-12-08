#ifndef SOLIDMECHTEMPCOUPLEX
#define SOLIDMECHTEMPCOUPLEX

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleX;

template<>
InputParameters validParams<SolidMechTempCoupleX>();

class SolidMechTempCoupleX : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleX(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};
#endif //SOLIDMECHTEMPCOUPLEX 
