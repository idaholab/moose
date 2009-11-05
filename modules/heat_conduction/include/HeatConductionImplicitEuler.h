#ifndef HEATCONDUCTIONIMPLICITEULER
#define HEATCONDUCTIONIMPLICITEULER

#include "ImplicitEuler.h"
#include "Material.h"


//Forward Declarations
class HeatConductionImplicitEuler;

class HeatConductionImplicitEuler : public ImplicitEuler
{
public:

  HeatConductionImplicitEuler(std::string name,
                              Parameters parameters,
                              std::string var_name,
                              std::vector<std::string> coupled_to=std::vector<std::string>(0),
                              std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  virtual void subdomainSetup();
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  std::vector<Real> * _specific_heat;
  std::vector<Real> * _density;
};
#endif //HEATCONDUCTIONIMPLICITEULER
