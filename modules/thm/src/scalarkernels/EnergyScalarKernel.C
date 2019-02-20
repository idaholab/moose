#include "EnergyScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", EnergyScalarKernel);

template <>
InputParameters
validParams<EnergyScalarKernel>()
{
  InputParameters params = validParams<NodalScalarKernel>();

  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("A", "Area");

  params.addRequiredParam<std::vector<Real>>("normals", "node normals");
  params.addRequiredCoupledVar("total_mfr_in", "Total mass flow rate into the junction");
  params.addRequiredCoupledVar("total_int_energy_rate_in",
                               "Total internal energy rate flowing into junction");

  return params;
}

EnergyScalarKernel::EnergyScalarKernel(const InputParameters & parameters)
  : NodalScalarKernel(parameters),
    _rhoA_var_number(coupled("rhoA")),
    _rhoA(coupledValue("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _rhoEA(coupledValue("rhoEA")),
    _vel(coupledValue("vel")),
    _area(coupledValue("A")),
    _normals(getParam<std::vector<Real>>("normals")),
    _total_mfr_in(coupledScalarValue("total_mfr_in")),
    _total_int_energy_rate_in(coupledScalarValue("total_int_energy_rate_in"))
{
}

void
EnergyScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  Real specific_thermal_energy_junction; // specific thermal(int) energy in this junction
  if (std::abs(_total_mfr_in[0]) < std::numeric_limits<Real>::min() * 100.0)
    specific_thermal_energy_junction = _rhoEA[0] / _rhoA[0] - 0.5 * _vel[0] * _vel[0];
  else
    specific_thermal_energy_junction = _total_int_energy_rate_in[0] / _total_mfr_in[0];
  re(0) = _u[0] - specific_thermal_energy_junction;
}

void
EnergyScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & kee = _assembly.jacobianBlock(_var.number(), _var.number());
  // d[Res(lamda_0)] / d(lamda_0) = 0.0;
  for (unsigned int i = 0; i < kee.m(); i++)
    kee(i, i) = 1.;

  // Off-diagonal blocks
  DenseMatrix<Number> & rhoA_ken = _assembly.jacobianBlock(_var.number(), _rhoA_var_number);
  DenseMatrix<Number> & rhouA_ken = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  DenseMatrix<Number> & rhoEA_ken = _assembly.jacobianBlock(_var.number(), _rhoEA_var_number);

  if (std::abs(_total_mfr_in[0]) < std::numeric_limits<Real>::min() * 100.0)
  {
    rhoA_ken(0, 0) += 1 / (_rhoA[0] * _rhoA[0]) * (_rhoEA[0] - _vel[0] * _rhouA[0]);
    rhouA_ken(0, 0) += _vel[0] / _rhoA[0];
    rhoEA_ken(0, 0) += -1 / _rhoA[0];
  }
  else
  {
    Real nmi2 = (_total_mfr_in[0] * _total_mfr_in[0]);
    for (unsigned int i = 0; i < _area.size(); i++)
    {
      if (_normals[i] * _vel[i] > 0) // case of net flow into the junction
      {
        Real u2 = _vel[i] * _vel[i];
        // derivative of _total_int_energy_rate_in w.r.t. conserved variables
        Real dtieri_darhoA = _normals[i] * _vel[i] * (u2 - _rhoEA[i] / _rhoA[i]);
        Real dtieri_darhouA = _normals[i] * (_rhoEA[i] / _rhoA[i] - 1.5 * u2);
        Real dtieri_darhoEA = _normals[i] * _vel[i];

        rhoA_ken(0, i) -= dtieri_darhoA / _total_mfr_in[0];
        rhouA_ken(0, i) -=
            (dtieri_darhouA * _total_mfr_in[0] - _total_int_energy_rate_in[0] * _normals[i]) / nmi2;
        rhoEA_ken(0, i) -= dtieri_darhoEA / _total_mfr_in[0];
      }
    }
  }
}
