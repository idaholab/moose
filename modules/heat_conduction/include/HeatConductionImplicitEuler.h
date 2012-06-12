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

  HeatConductionImplicitEuler(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  const MaterialProperty<Real> & _specific_heat;
  const MaterialProperty<Real> & _density;
};
#endif //HEATCONDUCTIONIMPLICITEULER
