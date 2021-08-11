#include "UpdatedLagrangianStressDivergence.h"

registerMooseObject("TensorMechanicsApp", UpdatedLagrangianStressDivergence);

InputParameters
UpdatedLagrangianStressDivergence::validParams()
{
  InputParameters params = LagrangianStressDivergenceBase::validParams();

  params.addClassDescription("Updated Lagrangian stress equilibrium kernel");

  return params;
}

UpdatedLagrangianStressDivergence::UpdatedLagrangianStressDivergence(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _avg_grad_trial(_grad_phi.size()),
    _uF(getMaterialPropertyByName<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _aF(getMaterialPropertyByName<RankTwoTensor>(_base_name + "avg_deformation_gradient")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "cauchy_stress")),
    _material_jacobian(getMaterialPropertyByName<RankFourTensor>(_base_name + "cauchy_jacobian")),
    _df(getMaterialPropertyByName<RankTwoTensor>(_base_name + "inv_inc_def_grad"))
{
  // This kernel requires used_displaced_mesh to be true if large kinematics
  // is on
  if (_large_kinematics && (!getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernel requires "
               "used_displaced_mesh = true for large_kinematics = true");

  // Similarly, if large kinematics is off so should use_displaced_mesh
  if (!_large_kinematics && (getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernel requires "
               "used_displaced_mesh = false for large_kinematics = false");
}

Real
UpdatedLagrangianStressDivergence::computeQpResidual()
{
  // Do the whole sum because future expansion to B_Bar would introduce
  // off-diagonal terms
  return _stress[_qp].doubleContraction(testGrad(_component));
}

Real
UpdatedLagrangianStressDivergence::computeQpJacobian()
{
  Real value = matJacobianComponent(_material_jacobian[_qp],
                                    testGrad(_component),
                                    trialGrad(_component, _stabilize_strain),
                                    _df[_qp]);

  if (_large_kinematics)
    value +=
        geomJacobianComponent(testGrad(_component), trialGrad(_component, false), _stress[_qp]);

  return value;
}

Real
UpdatedLagrangianStressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real value = 0.0;

  for (unsigned int cc = 0; cc < _ndisp; cc++)
  {
    if (jvar == _disp_nums[cc])
    {
      value += matJacobianComponent(_material_jacobian[_qp],
                                    testGrad(_component),
                                    trialGrad(cc, _stabilize_strain),
                                    _df[_qp]);
      if (_large_kinematics)
        value += geomJacobianComponent(testGrad(_component), trialGrad(cc, false), _stress[_qp]);
      break;
    }
  }

  return value;
}

void
UpdatedLagrangianStressDivergence::precalculateJacobian()
{
  if (_stabilize_strain)
    computeAverageGradTrial();
}

Real
UpdatedLagrangianStressDivergence::matJacobianComponent(const RankFourTensor & C,
                                                        const RankTwoTensor & grad_test,
                                                        const RankTwoTensor & grad_trial,
                                                        const RankTwoTensor & df)
{
  return grad_test.doubleContraction(C * (df * grad_trial));
}

Real
UpdatedLagrangianStressDivergence::geomJacobianComponent(const RankTwoTensor & grad_test,
                                                         const RankTwoTensor & grad_trial,
                                                         const RankTwoTensor & stress)
{
  return stress.doubleContraction(grad_test) * grad_trial.trace() -
         stress.doubleContraction(grad_test * grad_trial);
}

RankTwoTensor
UpdatedLagrangianStressDivergence::testGrad(unsigned int i)
{
  return gradOp(i, _grad_test[_i][_qp]);
}

void
UpdatedLagrangianStressDivergence::computeAverageGradTrial()
{
  avgGrad(_grad_phi, _avg_grad_trial);
}

RankTwoTensor
UpdatedLagrangianStressDivergence::trialGrad(unsigned int m, bool stabilize)
{
  // We need the switch here because the "geometric" part of the tangent
  // doesn't take the stabilization
  if (stabilize)
    return fullGrad(m, stabilize, _grad_phi[_j][_qp], _avg_grad_trial[_j]);
  // Don't need the average value
  else
    return fullGrad(m, stabilize, _grad_phi[_j][_qp], RealVectorValue());
}

void
UpdatedLagrangianStressDivergence::avgGrad(const VariablePhiGradient & grads,
                                           std::vector<RealVectorValue> & res)
{
  // Annoyingly this needs to be a integral over the reference
  // domain, like in the strain calculator...

  // Make sure we have enough space for all the gradients
  res.resize(grads.size());
  // Loop on shape function
  for (size_t beta = 0; beta < grads.size(); ++beta)
  {
    // Zero out
    res[beta].zero();
    Real v = 0.0;

    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      // We need to map back (and then forward) to do the right
      // average, first setup this backward map
      RankTwoTensor T = RankTwoTensor::Identity();
      if (_large_kinematics)
        T = _uF[_qp];
      Real J = T.det();

      // Accumulate the volume integral
      v += _JxW[_qp] * _coord[_qp] / J;

      // Map to reference
      RealVectorValue ref_grad = T.transpose() * grads[beta][_qp];
      // Accumulate
      res[beta] += _JxW[_qp] * _coord[_qp] * ref_grad / J;
    }
    // Average
    res[beta] /= v;

    // Map forward
    RankTwoTensor T = RankTwoTensor::Identity();
    if (_large_kinematics)
      T = _aF[0]; // All quad points have the same value
    res[beta] = T.inverse().transpose() * res[beta];
  }
}

RankTwoTensor
UpdatedLagrangianStressDivergence::stabilizeGrad(const RankTwoTensor & Gb, const RankTwoTensor & Ga)
{
  // Take the base gradient (Gb) and the average gradient (Ga) and
  // stabilize
  if (_large_kinematics)
    return (Gb - RankTwoTensor::Identity() * (Gb.trace() - Ga.trace()) / 3.0);

  // Small deformation variant
  return Gb + (Ga.trace() - Gb.trace()) / 3.0 * RankTwoTensor::Identity();
}
