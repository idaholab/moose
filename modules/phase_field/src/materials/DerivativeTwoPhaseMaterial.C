#include "DerivativeTwoPhaseMaterial.h"

template<>
InputParameters validParams<DerivativeTwoPhaseMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Two phase material that combines two single phase materials using a switching function.");

  // Two base materials
  params.addRequiredParam<std::string>("fa_name", "Phase A material (at eta=0)");
  params.addRequiredParam<std::string>("fb_name", "Phase A material (at eta=1)");
  params.addParam<std::string>("h", "h", "Switching Function Material that provides h(eta)");
  params.addParam<std::string>("g", "g", "Barrier Function Material that provides g(eta)");

  // Order parameter which determines the phase
  params.addCoupledVar("eta", "Order parameter");

  // Variables with applied tolerances and their tolerance values
  params.addParam<Real>("W", 0.0, "Energy barrier for the phase transformation from A to B");
  params.addParam<std::vector<Real> >("tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  return params;
}

InputParameters
DerivativeTwoPhaseMaterial::addPhiToArgs(InputParameters params)
{
  VariableName eta = params.set<std::vector<VariableName> >("eta")[0];
  params.set<std::vector<VariableName> >("args").push_back(eta);
  return params;
}

DerivativeTwoPhaseMaterial::DerivativeTwoPhaseMaterial(const std::string & name,
                                                       InputParameters parameters) :
    DerivativeBaseMaterial(name, addPhiToArgs(parameters)),
    _eta(coupledValue("eta")),
    _eta_name(getVar("eta", 0)->name()),
    _eta_id(_nargs - 1),
    _fa_name(getParam<std::string>("fa_name")),
    _fb_name(getParam<std::string>("fb_name")),
    _h_name(getParam<std::string>("h")),
    _h(getMaterialProperty<Real>(_h_name)),
    _dh(getMaterialPropertyDerivative<Real>(_h_name, _eta_name)),
    _d2h(getMaterialPropertyDerivative<Real>(_h_name, _eta_name, _eta_name)),
    _g_name(getParam<std::string>("g")),
    _g(getMaterialProperty<Real>(_h_name)),
    _dg(getMaterialPropertyDerivative<Real>(_g_name, _eta_name)),
    _d2g(getMaterialPropertyDerivative<Real>(_g_name, _eta_name, _eta_name)),
    _W(getParam<Real>("W")),
    _prop_Fa(getMaterialProperty<Real>(_fa_name)),
    _prop_Fb(getMaterialProperty<Real>(_fb_name))
{
  // eta is appended to args to have the base class add all the derivative material properties containing eta
  // however we treat derivatives w.r.t. eta differently. They are computed from the composite form only, as
  // Fa and Fb are not functions of eta.
  _nfargs = _nargs - 1;

  // reserve space for phase A and B material properties
  _prop_dFa.resize(_nfargs);
  _prop_d2Fa.resize(_nfargs);
  _prop_d3Fa.resize(_nfargs);
  _prop_dFb.resize(_nfargs);
  _prop_d2Fb.resize(_nfargs);
  _prop_d3Fb.resize(_nfargs);
  for (unsigned int i = 0; i < _nfargs; ++i)
  {
    _prop_dFa[i] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i]);
    _prop_dFb[i] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i]);

    _prop_d2Fa[i].resize(_nfargs);
    _prop_d2Fb[i].resize(_nfargs);

    // TODO: maybe we should reserve and initialize to NULL...
    if (_third_derivatives) {
      _prop_d3Fa[i].resize(_nfargs);
      _prop_d3Fb[i].resize(_nfargs);
    }

    for (unsigned int j = 0; j < _nfargs; ++j)
    {
      _prop_d2Fa[i][j] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i], _arg_names[j]);
      _prop_d2Fb[i][j] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i], _arg_names[j]);

      if (_third_derivatives) {
        _prop_d3Fa[i][j].resize(_nfargs);
        _prop_d3Fb[i][j].resize(_nfargs);

        for (unsigned int k = 0; k < _nfargs; ++k)
        {
          _prop_d3Fa[i][j][k] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i], _arg_names[j], _arg_names[k]);
          _prop_d3Fb[i][j][k] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

/// Fm(cmg,cmv,T) takes three arguments
unsigned int
DerivativeTwoPhaseMaterial::expectedNumArgs()
{
  // this always returns the number of arguments that was passed in
  // i.e. any number of args is accepted.
  return _nargs;
}

Real
DerivativeTwoPhaseMaterial::computeF()
{
  return _h[_qp] * _prop_Fb[_qp] + (1.0 - _h[_qp]) * _prop_Fa[_qp] + _W * _g[_qp];
}

Real
DerivativeTwoPhaseMaterial::computeDF(unsigned int i)
{
  if (i == _eta_id)
    return _dh[_qp] * (_prop_Fb[_qp] - _prop_Fa[_qp]) + _W * _dg[_qp];
  else
    return _h[_qp] * (*_prop_dFb[i])[_qp] + (1.0 - _h[_qp]) * (*_prop_dFa[i])[_qp];
}

Real
DerivativeTwoPhaseMaterial::computeD2F(unsigned int i, unsigned int j)
{
  if (i == _eta_id && j == _eta_id)
    return _d2h[_qp] * (_prop_Fb[_qp] - _prop_Fa[_qp]) + _W * _d2g[_qp];

  if (i == _eta_id)
    return _dh[_qp] * ((*_prop_dFb[j])[_qp] - (*_prop_dFa[j])[_qp]);
  if (j == _eta_id)
    return _dh[_qp] * ((*_prop_dFb[i])[_qp] - (*_prop_dFa[i])[_qp]);

  return _h[_qp] * (*_prop_d2Fb[i][j])[_qp] + (1.0 - _h[_qp]) * (*_prop_d2Fa[i][j])[_qp];
}
