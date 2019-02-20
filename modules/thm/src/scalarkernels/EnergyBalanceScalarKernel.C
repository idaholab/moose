#include "EnergyBalanceScalarKernel.h"
#include "SinglePhaseFluidProperties.h"
#include "Assembly.h"
#include "Numerics.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", EnergyBalanceScalarKernel);

template <>
InputParameters
validParams<EnergyBalanceScalarKernel>()
{
  InputParameters params = validParams<JunctionScalarKernel>();
  params.addRequiredCoupledVar("rhoA", "Density * Area");
  params.addRequiredCoupledVar("rhouA", "Momentum * Area");
  params.addRequiredCoupledVar("rhoEA", "Total Energy * Area");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("p", "Coupled pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties user object to use.");
  return params;
}

EnergyBalanceScalarKernel::EnergyBalanceScalarKernel(const InputParameters & parameters)
  : JunctionScalarKernel(parameters),
    _rhoA_var_number(coupled("rhoA")),
    _rhoA(coupledValue("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _rhoEA(coupledValue("rhoEA")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _vel(coupledValue("vel")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

void
EnergyBalanceScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (unsigned int i = 0; i < _rhoEA.size(); i++)
    re(0) -= _normals[i] * _vel[i] *
             (_rhoEA[i] + _pressure[i] * _area[i] - 0.5 * _rhoA[i] * _vel[i] * _vel[i]);
}

void
EnergyBalanceScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & rho_ken = _assembly.jacobianBlock(_var.number(), _rhoA_var_number);
  DenseMatrix<Number> & rhou_ken = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  DenseMatrix<Number> & rhoE_ken = _assembly.jacobianBlock(_var.number(), _rhoEA_var_number);

  for (unsigned int i = 0; i < _area.size(); i++)
  {
    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(_v[i], _e[i], p, dp_dv, dp_de);

    Real dv_drhoA = THM::dv_darhoA(_area[i], _rhoA[i]);
    Real de_drhoA = THM::de_darhoA(_rhoA[i], _rhouA[i], _rhoEA[i]);
    Real de_drhouA = THM::de_darhouA(_rhoA[i], _rhouA[i]);
    Real de_drhoEA = THM::de_darhoEA(_rhoA[i]);

    Real dp_drhoA = dp_dv * dv_drhoA + dp_de * de_drhoA;
    Real dp_drhouA = dp_de * de_drhouA;
    Real dp_drhoEA = dp_de * de_drhoEA;

    Real u2 = _vel[i] * _vel[i];
    rho_ken(0, i) -=
        _normals[i] * _vel[i] *
        (-_rhoEA[i] / _rhoA[i] + dp_drhoA * _area[i] - _pressure[i] * _area[i] / _rhoA[i] + u2);
    rhou_ken(0, i) -= _normals[i] * (_rhoEA[i] / _rhoA[i] + _pressure[i] * _area[i] / _rhoA[i] +
                                     _vel[i] * dp_drhouA * _area[i] - 1.5 * u2);
    rhoE_ken(0, i) -= _normals[i] * _vel[i] * (1.0 + dp_drhoEA * _area[i]);
  }
}
