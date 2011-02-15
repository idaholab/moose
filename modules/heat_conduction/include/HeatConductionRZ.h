#ifndef HEATCONDUCTIONRZ_H
#define HEATCONDUCTIONRZ_H

#include "HeatConduction.h"

//Forward Declarations
class HeatConductionRZ;

template<>
InputParameters validParams<HeatConductionRZ>();

class HeatConductionRZ : public HeatConduction
{
public:

  HeatConductionRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

};
#endif //HEATCONDUCTIONRZ_H
