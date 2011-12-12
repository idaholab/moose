#include "HeatConductionImplicitEulerRZ.h"

template<>
InputParameters validParams<HeatConductionImplicitEulerRZ>()
{
  InputParameters params = validParams<HeatConductionImplicitEuler>();
  return params;
}


HeatConductionImplicitEulerRZ::HeatConductionImplicitEulerRZ(const std::string & name,
                                                             InputParameters parameters)
  :HeatConductionImplicitEuler(name, parameters)
{}

Real
HeatConductionImplicitEulerRZ::computeQpResidual()
{
  return HeatConductionImplicitEuler::computeQpResidual();
}

Real
HeatConductionImplicitEulerRZ::computeQpJacobian()
{
  return HeatConductionImplicitEuler::computeQpJacobian();
}

Real
HeatConductionImplicitEulerRZ::computeQpOffDiagJacobian( unsigned jvar )
{
  return HeatConductionImplicitEuler::computeQpOffDiagJacobian( jvar );
}
