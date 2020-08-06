#include "CurrentDensity.h"

registerMooseObject("ElkApp", CurrentDensity);
registerMooseObject("ElkApp", ADCurrentDensity);

template <bool is_ad>
InputParameters
CurrentDensityTempl<is_ad>::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("Calculates the current density vector field when given electrostatic potential (electrostatic = true, default) or electric field.");
  params.addParam<bool>("electrostatic", true, "Whether the electric field is based on electrostatic potential or is fully electromagnetic (default = TRUE)");
  params.addCoupledVar("potential", "Electrostatic potential variable");
  params.addCoupledVar("electric_field", "Electric field variable (electromagnetic)");
  return params;
}

template <bool is_ad>
CurrentDensityTempl<is_ad>::CurrentDensityTempl(const InputParameters & parameters)
  : VectorAuxKernel(parameters),

  _is_es(getParam<bool>("electrostatic")),
  _grad_potential(coupledGradient("potential")),
  _electric_field(coupledVectorValue("electric_field")),

  _conductivity(getGenericMaterialProperty<Real, is_ad>("electrical_conductivity"))
{
}

template <bool is_ad>
RealVectorValue
CurrentDensityTempl<is_ad>::computeValue()
{
  if (_is_es)
    return MetaPhysicL::raw_value(_conductivity[_qp]) * -_grad_potential[_qp];
  else
    return MetaPhysicL::raw_value(_conductivity[_qp]) * _electric_field[_qp];

}

template class CurrentDensityTempl<false>;
template class CurrentDensityTempl<true>;
