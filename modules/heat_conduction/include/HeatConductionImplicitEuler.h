#ifndef HEATCONDUCTIONIMPLICITEULER
#define HEATCONDUCTIONIMPLICITEULER

#include "ImplicitEuler.h"
#include "Material.h"

//Forward Declarations
class HeatConductionImplicitEuler;

template<>
InputParameters validParams<HeatConductionImplicitEuler>();

class HeatConductionImplicitEuler : public ImplicitEuler
{
public:

  HeatConductionImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();
  
private:
  MooseArray<Real> & _specific_heat;
  MooseArray<Real> & _density;
};
#endif //HEATCONDUCTIONIMPLICITEULER
