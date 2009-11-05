#ifndef SOLIDMECHTEMPCOUPLEZ
#define SOLIDMECHTEMPCOUPLEZ

#include "SolidMechTempCouple.h"


//Forward Declarations
class SolidMechTempCoupleZ;

template<>
Parameters valid_params<SolidMechTempCoupleZ>();

class SolidMechTempCoupleZ : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleZ(std::string name,
                       Parameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};
 
#endif //SOLIDMECHTEMPCOUPLEZ
