#ifndef SOLIDMECHTEMPCOUPLEY
#define SOLIDMECHTEMPCOUPLEY

#include "SolidMechTempCouple.h"


//Forward Declarations
class SolidMechTempCoupleY;

template<>
Parameters valid_params<SolidMechTempCoupleY>();

class SolidMechTempCoupleY : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleY(std::string name,
                       Parameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
};
 
#endif //SOLIDMECHTEMPCOUPLEY
