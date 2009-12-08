#ifndef HEATCONDUCTION_H
#define HEATCONDUCTION_H

#include "Diffusion.h"
#include "Material.h"


//Forward Declarations
class HeatConduction;

template<>
InputParameters validParams<HeatConduction>();

class HeatConduction : public Diffusion
{
public:

  HeatConduction(std::string name,
                 InputParameters parameters,
                 std::string var_name,
                 std::vector<std::string> coupled_to=std::vector<std::string>(0),
                 std::vector<std::string> coupled_as=std::vector<std::string>(0));

  virtual void subdomainSetup();
  
protected:  
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
private:
  std::vector<Real> * _k;
};
#endif //HEATCONDUCTION_H
