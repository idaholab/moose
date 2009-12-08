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

  SolidMechTempCoupleX(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};
#endif //SOLIDMECHTEMPCOUPLEX 
