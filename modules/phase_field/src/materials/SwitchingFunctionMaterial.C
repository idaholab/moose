#include "SwitchingFunctionMaterial.h"

template<>
InputParameters validParams<SwitchingFunctionMaterial>()
{
  InputParameters params = validParams<OrderParameterFunctionMaterial>();
  params.addClassDescription("Helper material to provide h(eta) and its derivative in one of two polynomial forms.\nSIMPLE: 3*eta^2-2*eta^3\nHIGH: eta^3*(6*eta^2-15*eta+10)");
  MooseEnum h_order("SIMPLE=0 HIGH", "SIMPLE");
  params.addParam<MooseEnum>("h_order", h_order, "Polynomial order of the switching function h(eta)");
  params.set<std::string>("function_name") = std::string("h");
  return params;
}

SwitchingFunctionMaterial::SwitchingFunctionMaterial(const std::string & name,
                                                     InputParameters parameters) :
    OrderParameterFunctionMaterial(name, parameters),
    _h_order(getParam<MooseEnum>("h_order"))
{
}

void
SwitchingFunctionMaterial::computeQpProperties()
{
  switch (_h_order)
  {
    case 0: // SIMPLE
      _prop_f[_qp] =   3.0 * _eta[_qp]*_eta[_qp]
                     - 2.0 * _eta[_qp]*_eta[_qp]*_eta[_qp];
      _prop_df[_qp] =   6.0 * _eta[_qp]
                      - 6.0 * _eta[_qp]*_eta[_qp];
      _prop_d2f[_qp] = 6.0 - 12.0 * _eta[_qp];
      break;

    case 1: // HIGH
      _prop_f[_qp] =   _eta[_qp]*_eta[_qp]*_eta[_qp]
                     * (6.0 * _eta[_qp]*_eta[_qp] - 15.0 * _eta[_qp] + 10.0);
      _prop_df[_qp] =   30.0 * _eta[_qp]*_eta[_qp]
                      * (_eta[_qp]*_eta[_qp] - 2.0 * _eta[_qp] + 1.0);
      _prop_d2f[_qp] = _eta[_qp] * (120.0 * _eta[_qp]*_eta[_qp] - 180.0 * _eta[_qp] + 60.0);
      break;

    default:
      mooseError("Internal error");
  }
}
