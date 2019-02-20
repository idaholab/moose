#include "OneDEnergyMassFlowRateTemperatureBC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", OneDEnergyMassFlowRateTemperatureBC);

template <>
InputParameters
validParams<OneDEnergyMassFlowRateTemperatureBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<Real>("m_dot", "The specified mass flow rate value.");
  params.addRequiredParam<Real>("T", "The specified temperature value.");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure property name");
  params.addRequiredCoupledVar("arhoA", "Conserved density of the phase.");
  params.addRequiredCoupledVar("arhouA", "Conserved momentum of the phase.");
  params.addRequiredCoupledVar("arhoEA", "Conserved total energy of the phase.");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  params.declareControllable("m_dot T");

  return params;
}

OneDEnergyMassFlowRateTemperatureBC::OneDEnergyMassFlowRateTemperatureBC(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _m_dot(getParam<Real>("m_dot")),
    _T(getParam<Real>("T")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),

    _area(coupledValue("A")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),
    _dp_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("p", "beta") : nullptr),
    _arhoA_var_num(coupled("arhoA")),
    _arhouA_var_num(coupled("arhouA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
OneDEnergyMassFlowRateTemperatureBC::computeQpResidual()
{
  Real arhouA = _m_dot;
  Real rho = _fp.rho_from_p_T(_p[_qp], _T);
  Real e = _fp.e_from_p_rho(_p[_qp], rho);
  Real u = arhouA / _alpha[_qp] / rho / _area[_qp];
  Real E = e + 0.5 * u * u;
  return (arhouA * E + _alpha[_qp] * u * _p[_qp] * _area[_qp]) * _normal * _test[_i][_qp];
}

Real
OneDEnergyMassFlowRateTemperatureBC::computeQpJacobian()
{
  Real rho, drho_dp, drho_dT;
  _fp.rho_from_p_T(_p[_qp], _T, rho, drho_dp, drho_dT);

  Real arhouA = _m_dot;
  Real drho_darhoEA = drho_dp * _dp_darhoEA[_qp];

  Real u = arhouA / _alpha[_qp] / rho / _area[_qp];
  Real du_darhoEA = -arhouA / _alpha[_qp] / _area[_qp] / rho / rho * drho_darhoEA;

  Real e, de_dp, de_drho;
  _fp.e_from_p_rho(_p[_qp], rho, e, de_dp, de_drho);
  Real de_darhoEA = de_dp * _dp_darhoEA[_qp] + de_drho * drho_darhoEA;
  Real dE_darhoEA = de_darhoEA + u * du_darhoEA;

  return (arhouA * dE_darhoEA +
          _alpha[_qp] * (du_darhoEA * _p[_qp] + u * _dp_darhoEA[_qp]) * _area[_qp]) *
         _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDEnergyMassFlowRateTemperatureBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_num)
  {
    Real rho, drho_dp, drho_dT;
    _fp.rho_from_p_T(_p[_qp], _T, rho, drho_dp, drho_dT);

    Real arhouA = _m_dot;
    Real drho_darhoA = drho_dp * _dp_darhoA[_qp];

    Real u = arhouA / _alpha[_qp] / rho / _area[_qp];
    Real du_darhoA = -arhouA / _alpha[_qp] / _area[_qp] / rho / rho * drho_darhoA;

    Real e, de_dp, de_drho;
    _fp.e_from_p_rho(_p[_qp], rho, e, de_dp, de_drho);
    Real de_darhoA = de_dp * _dp_darhoA[_qp] + de_drho * drho_darhoA;
    Real dE_darhoA = de_darhoA + u * du_darhoA;

    return (arhouA * dE_darhoA +
            _alpha[_qp] * (du_darhoA * _p[_qp] + u * _dp_darhoA[_qp]) * _area[_qp]) *
           _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_num)
  {
    Real rho, drho_dp, drho_dT;
    _fp.rho_from_p_T(_p[_qp], _T, rho, drho_dp, drho_dT);

    Real arhouA = _m_dot;
    Real drho_darhouA = drho_dp * _dp_darhouA[_qp];

    Real u = arhouA / _alpha[_qp] / rho / _area[_qp];
    Real du_darhouA = -arhouA / _alpha[_qp] / _area[_qp] / rho / rho * drho_darhouA;

    Real e, de_dp, de_drho;
    _fp.e_from_p_rho(_p[_qp], rho, e, de_dp, de_drho);
    Real de_darhouA = de_dp * _dp_darhouA[_qp] + de_drho * drho_darhouA;
    Real dE_darhouA = de_darhouA + u * du_darhouA;

    return (arhouA * dE_darhouA +
            _alpha[_qp] * (du_darhouA * _p[_qp] + u * _dp_darhouA[_qp]) * _area[_qp]) *
           _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_num)
  {
    Real rho, drho_dp, drho_dT;
    _fp.rho_from_p_T(_p[_qp], _T, rho, drho_dp, drho_dT);

    Real arhouA = _m_dot;
    Real drho_dbeta = drho_dp * (*_dp_dbeta)[_qp];

    Real u = arhouA / _alpha[_qp] / rho / _area[_qp];
    Real du_dbeta = -arhouA / _area[_qp] *
                    ((*_dalpha_dbeta)[_qp] * rho + _alpha[_qp] * drho_dbeta) / _alpha[_qp] /
                    _alpha[_qp] / rho / rho;

    Real e, de_dp, de_drho;
    _fp.e_from_p_rho(_p[_qp], rho, e, de_dp, de_drho);
    Real de_dbeta = de_dp * (*_dp_dbeta)[_qp] + de_drho * drho_dbeta;
    Real dE_dbeta = de_dbeta + u * du_dbeta;

    return (arhouA * dE_dbeta +
            ((*_dalpha_dbeta)[_qp] * u * _p[_qp] + _alpha[_qp] * du_dbeta * _p[_qp] +
             _alpha[_qp] * u * (*_dp_dbeta)[_qp]) *
                _area[_qp]) *
           _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
