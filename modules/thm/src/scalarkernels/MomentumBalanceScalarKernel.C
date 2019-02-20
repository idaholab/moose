#include "MomentumBalanceScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", MomentumBalanceScalarKernel);

template <>
InputParameters
validParams<MomentumBalanceScalarKernel>()
{
  InputParameters params = validParams<JunctionScalarKernel>();
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredCoupledVar("rhoEA", "Coupled total energy");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("junction_rho", "Coupled value of rho from the junction");
  params.addRequiredParam<Real>("ref_area", "Reference area of this junction");
  params.addRequiredCoupledVar("total_mfr_in", "Total mass flow rate into the junction");
  params.declareControllable("ref_area");
  return params;
}

MomentumBalanceScalarKernel::MomentumBalanceScalarKernel(const InputParameters & parameters)
  : JunctionScalarKernel(parameters),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _vel(coupledValue("vel")),
    _junction_rho(coupledScalarValue("junction_rho")),
    _junction_rho_var_number(coupledScalar("junction_rho")),
    _total_mfr_in(coupledScalarValue("total_mfr_in")),
    _ref_area(getParam<Real>("ref_area"))
{
}

void
MomentumBalanceScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  Real ref_vel = _total_mfr_in[0] / _junction_rho[0] / _ref_area;
  re(0) += _u[0] - ref_vel;
}

void
MomentumBalanceScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & kee = _assembly.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & rho_ken = _assembly.jacobianBlock(_var.number(), _rhoA_var_number);
  DenseMatrix<Number> & rhou_ken = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  DenseMatrix<Number> & rhoE_ken = _assembly.jacobianBlock(_var.number(), _rhoEA_var_number);
  DenseMatrix<Number> & junction_rho_ken =
      _assembly.jacobianBlock(_var.number(), _junction_rho_var_number);

  kee(0, 0) += 1.;
  for (unsigned int i = 0; i < _vel.size(); i++)
  {
    rho_ken(0, i) += 0.0;
    if (_normals[i] * _vel[i] > 0) // case of net flow into the junction
      rhou_ken(0, i) += -_normals[i] / (_junction_rho[0] * _ref_area);
    else
      rhou_ken(0, i) += 0.0;
    rhoE_ken(0, i) += 0.0;
  }
  junction_rho_ken(0, 0) += _total_mfr_in[0] / _ref_area / _junction_rho[0] / _junction_rho[0];
}
