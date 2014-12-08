#include "BarrierFunctionMaterial.h"

template<>
InputParameters validParams<BarrierFunctionMaterial>()
{
  InputParameters params = validParams<OrderParameterFunctionMaterial>();
  params.addClassDescription("Helper material to provide g(eta) and its derivative in a polynomial.\nSIMPLE: eta^2*(1-eta)^2\nLOW: eta*(1-eta)");
  MooseEnum h_order("SIMPLE=0 LOW", "SIMPLE");
  params.addParam<MooseEnum>("g_order", h_order, "Polynomial order of the switching function h(eta)");
  params.set<std::string>("function_name") = std::string("g");
  return params;
}

BarrierFunctionMaterial::BarrierFunctionMaterial(const std::string & name,
                                                 InputParameters parameters) :
    OrderParameterFunctionMaterial(name, parameters),
    _g_order(getParam<MooseEnum>("g_order"))
{
}

void
BarrierFunctionMaterial::computeQpProperties()
{
  switch (_g_order)
  {
    case 0: // SIMPLE
      _prop_f[_qp] =   _eta[_qp] * _eta[_qp]
                     * (1.0 - _eta[_qp]) * (1.0 - _eta[_qp]);
      _prop_df[_qp] =   2.0 * _eta[_qp] * (_eta[_qp] - 1.0)
                      * (2.0 * _eta[_qp] - 1.0);
      _prop_d2f[_qp] = 12.0 * (_eta[_qp] * _eta[_qp] - _eta[_qp]) + 2.0;
      break;

    case 1: // LOW
      _prop_f[_qp] = _eta[_qp] * (1.0 - _eta[_qp]);
      _prop_df[_qp] = 1.0 - 2.0 * _eta[_qp];
      _prop_d2f[_qp] = - 2.0;
      break;

    default:
      mooseError("Internal error");
  }
}
