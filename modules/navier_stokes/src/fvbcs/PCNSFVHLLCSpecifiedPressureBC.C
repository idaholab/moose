#include "PCNSFVHLLCSpecifiedPressureBC.h"
#include "NS.h"
#include "Function.h"

InputParameters
PCNSFVHLLCSpecifiedPressureBC::validParams()
{
  auto params = PCNSFVHLLCBC::validParams();
  params.addRequiredParam<FunctionName>(NS::pressure, "A function for the pressure");
  return params;
}

PCNSFVHLLCSpecifiedPressureBC::PCNSFVHLLCSpecifiedPressureBC(const InputParameters & parameters)
  : PCNSFVHLLCBC(parameters), _pressure_boundary_function(getFunction(NS::pressure))
{
}

void
PCNSFVHLLCSpecifiedPressureBC::preComputeWaveSpeed()
{
  _pressure_boundary = _pressure_boundary_function.value(_t, _face_info->faceCentroid());
  _eps_boundary = _eps_elem[_qp];

  // rho and vel implicit -> 1 + n_dim numerical bcs
  _rho_boundary = _rho_elem[_qp];
  _vel_boundary = _vel_elem[_qp];

  _normal_speed_boundary = _normal * _vel_boundary;
  _specific_internal_energy_boundary = _fluid.e_from_p_rho(_pressure_boundary, _rho_boundary);
  _et_boundary = _specific_internal_energy_boundary + 0.5 * _vel_boundary * _vel_boundary;
  _rho_et_boundary = _rho_boundary * _et_boundary;
  _ht_boundary = _et_boundary + _pressure_boundary / _rho_boundary;
}
