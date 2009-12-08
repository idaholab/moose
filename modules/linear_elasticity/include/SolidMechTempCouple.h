#ifndef SOLIDMECHTEMPCOUPLE_H
#define SOLIDMECHTEMPCOUPLE_H

#include "SolidMech.h"

//libMesh includes
#include "tensor_value.h"


//Forward Declarations
class SolidMechTempCouple;

template<>
InputParameters validParams<SolidMechTempCouple>();

class SolidMechTempCouple : public SolidMech
{
public:

  SolidMechTempCouple(std::string name,
                      InputParameters parameters,
                      std::string var_name,
                      std::vector<std::string> coupled_to,
                      std::vector<std::string> coupled_as);
  
  virtual void subdomainSetup();
  
  void recomputeCouplingConstants();
  
protected:
  unsigned int _temp_var;
  
  std::vector<Real> * _thermal_strain;
  std::vector<Real> * _alpha;

  Real _c4;
};
 

#endif //SOLIDMECHTEMPCOUPLE_H
