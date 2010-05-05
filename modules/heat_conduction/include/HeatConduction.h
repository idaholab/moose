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

  HeatConduction(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual void subdomainSetup();
  
protected:  
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
private:
  std::vector<Real> * _k;
};
#endif //HEATCONDUCTION_H
