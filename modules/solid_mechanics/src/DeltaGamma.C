#include "DeltaGamma.h"

#include "Material.h"

template<>
InputParameters validParams<DeltaGamma>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


DeltaGamma::DeltaGamma(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _elastic_strain(getColumnMajorMatrixMaterialProperty("elastic_strain")),
   _accumulated_plastic_strain(getRealMaterialProperty("accumulated_plastic_strain")),
   _von_mises_stress(getRealMaterialProperty("von_mises_stress")),
   _yield_stress(getRealMaterialProperty("yield_stress")),
   _shear_modulus(getRealMaterialProperty("shear_modulus"))
{}

Real
DeltaGamma::computeQpResidual()
{
  if(_von_mises_stress[_qp] > _yield_stress[_qp] + (_accumulated_plastic_strain[_qp] + _u[_qp]) * 100e9)
    return _phi[_i][_qp] * (_von_mises_stress[_qp] - (3 * _shear_modulus[_qp] * _u[_qp]) - (_yield_stress[_qp] + (_accumulated_plastic_strain[_qp] + _u[_qp]) * 100e9));

  return _phi[_i][_qp]*(_u[_qp] - 0);
}

Real
DeltaGamma::computeQpJacobian()
{
  if(_von_mises_stress[_qp] - _yield_stress[_qp] > 0.0)
    return _phi[_i][_qp] * ((3 * _shear_modulus[_qp] * _phi[_j][_qp]) - (_phi[_j][_qp] * 100e9));

  return _phi[_i][_qp]*_phi[_j][_qp];
}
