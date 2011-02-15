#ifndef HEATCONDUCTIONIMPLICITEULERRZ
#define HEATCONDUCTIONIMPLICITEULERRZ

#include "HeatConductionImplicitEuler.h"

//Forward Declarations
class HeatConductionImplicitEulerRZ;

template<>
InputParameters validParams<HeatConductionImplicitEulerRZ>();

class HeatConductionImplicitEulerRZ : public HeatConductionImplicitEuler
{
public:

  HeatConductionImplicitEulerRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

};
#endif //HEATCONDUCTIONIMPLICITEULERRZ
