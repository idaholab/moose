#include "TotalLagrangianStressDivergence.h"

registerMooseObject("TensorMechanicsApp", TotalLagrangianStressDivergence);

InputParameters
TotalLagrangianStressDivergence::validParams()
{
  InputParameters params = LagrangianStressDivergenceBase::validParams();

  params.addClassDescription("Total Lagrangian stress equilibrium kernel");

  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");

  return params;
}

TotalLagrangianStressDivergence::TotalLagrangianStressDivergence(const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _pk1(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _dpk1(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian")),
    _avg_grad_trial(_grad_phi.size()),
    _unstabilized_def_grad(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _aF(getMaterialPropertyByName<RankTwoTensor>(_base_name + "avg_deformation_gradient"))
{
}

Real
TotalLagrangianStressDivergence::computeQpResidual()
{
  return _pk1[_qp].doubleContraction(testGrad(_component));
}

Real
TotalLagrangianStressDivergence::computeQpJacobian()
{
  return materialJacobian(testGrad(_component), trialGrad(_component));
}

Real
TotalLagrangianStressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int cc = 0; cc < _ndisp; cc++)
    if (jvar == _disp_nums[cc])
      return materialJacobian(testGrad(_component), trialGrad(cc));

  return 0;
}

void
TotalLagrangianStressDivergence::precalculateJacobian()
{
  if (_stabilize_strain)
    computeAverageGradPhi();
}

RankTwoTensor
TotalLagrangianStressDivergence::stabilizeGrad(const RankTwoTensor & Gb, const RankTwoTensor & Ga)
{
  // Take the base gradient (Gb) and the average gradient (Ga) and
  // stabilize

  // Stabilization depends on the kinematics
  if (_large_kinematics)
  {
    // Horrible thing, see the documentation for how we get here
    const Real dratio = std::pow(_aF[_qp].det() / _unstabilized_def_grad[_qp].det(), 1.0 / 3.0);
    const Real fact = (_aF[_qp].inverse().transpose().doubleContraction(Ga) -
                       _unstabilized_def_grad[_qp].inverse().transpose().doubleContraction(Gb)) /
                      3.0;
    return dratio * (Gb + fact * _unstabilized_def_grad[_qp]);
  }
  // Small strain modification is linear
  return Gb + (Ga.trace() - Gb.trace()) / 3.0 * RankTwoTensor::Identity();
}

Real
TotalLagrangianStressDivergence::materialJacobian(const RankTwoTensor & grad_test,
                                                  const RankTwoTensor & grad_trial)
{
  return grad_test.doubleContraction(_dpk1[_qp] * grad_trial);
}

RankTwoTensor
TotalLagrangianStressDivergence::testGrad(unsigned int i)
{
  return gradOp(i, _grad_test[_i][_qp]);
}

RankTwoTensor
TotalLagrangianStressDivergence::trialGrad(unsigned int k)
{
  // The trick here is for the standard solids formulation you can work
  // with trial function gradient vectors (i.e. don't worry about the
  // other displacement components).  However for the
  // stabilized methods the "trace" term introduces non-zeros on
  // k indices other than the current trial function index...
  if (_stabilize_strain)
    return fullGrad(k, _stabilize_strain, _grad_phi[_j][_qp], _avg_grad_trial[_j]);
  // Don't need the averaged value
  else
    return fullGrad(k, _stabilize_strain, _grad_phi[_j][_qp], RealVectorValue());
}

void
TotalLagrangianStressDivergence::computeAverageGradPhi()
{
  // Use the generic "averager" to make the average
  avgGrad(_grad_phi, _avg_grad_trial);
}

void
TotalLagrangianStressDivergence::avgGrad(const VariablePhiGradient & grads,
                                         std::vector<RealVectorValue> & res)
{
  // Average a gradient and store in res
  // This function does the averaging over the reference configuration,
  // so it's quite straightforward
  res.resize(grads.size());
  for (unsigned int beta = 0; beta < grads.size(); ++beta)
  {
    // Zero
    res[beta].zero();
    Real v = 0.0;

    // Sum over qp
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      v += _JxW[_qp] * _coord[_qp];
      res[beta] += _JxW[_qp] * _coord[_qp] * grads[beta][_qp];
    }

    // Divide by the volume
    res[beta] /= v;
  }
}
