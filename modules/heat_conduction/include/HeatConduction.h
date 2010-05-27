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

protected:  
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();
  
private:
  MooseArray<Real> & _k;
};
#endif //HEATCONDUCTION_H
