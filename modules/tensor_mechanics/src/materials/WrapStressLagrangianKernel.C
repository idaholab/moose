#include "WrapStressLagrangianKernel.h"

registerMooseObject("TensorMechanicsApp", WrapStressLagrangianKernel);

InputParameters
WrapStressLagrangianKernel::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<bool>(
      "kernel_large_kinematics", false, "Use large displacement kinematics in the kernel.");
  params.addParam<bool>("material_large_kinematics",
                        false,
                        "If true the material returns the Cauchy stress"
                        " and the derivative with respect to the spatial "
                        "velocity gradient directly.");
  params.addCoupledVar("macro_gradient", "Optional scalar field with the macro gradient");
  return params;
}

WrapStressLagrangianKernel::WrapStressLagrangianKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _ld_kernel(getParam<bool>("kernel_large_kinematics")),
    _ld_material(getParam<bool>("material_large_kinematics")),
    _cauchy_stress(declareProperty<RankTwoTensor>("cauchy_stress")),
    _cauchy_stress_old(getMaterialPropertyOld<RankTwoTensor>("cauchy_stress")),
    _material_jacobian(declareProperty<RankFourTensor>("material_jacobian")),
    _df(getMaterialPropertyByName<RankTwoTensor>("inc_def_grad")),
    _stress_moose(getMaterialPropertyByName<RankTwoTensor>("stress")),
    _stress_moose_old(getMaterialPropertyOld<RankTwoTensor>("stress")),
    _jacobian_moose(getMaterialPropertyByName<RankFourTensor>("Jacobian_mult")),
    _PK1(declareProperty<RankTwoTensor>("PK1")),
    _J(getMaterialPropertyByName<Real>("detJ")),
    _F_inv(getMaterialPropertyByName<RankTwoTensor>("inv_def_grad"))
{
}

void
WrapStressLagrangianKernel::initQpStatefulProperties()
{
  _cauchy_stress[_qp].zero();
  _material_jacobian[_qp].zero();
  _PK1[_qp].zero();
}

void
WrapStressLagrangianKernel::computeQpProperties()
{
  // This should be implemented but I'll need to figure out
  // exactly what the heck the MOOSE large deformation materials pass back
  if ((_ld_kernel) && (_ld_material))
  {
    mooseError("Lagrangian kernel large deformations with MOOSE material large"
               " deformations is not implemented");
  }
  // Translation: I'm giving you the small strain material update, please
  // do the rotations for me.
  else if ((_ld_kernel) && (!_ld_material))
  {
    // We need to make an objective integration of the stress and the
    // tangent coming out of the MOOSE material system
    _objectiveUpdate();
  }
  // I don't think this case makes sense but maybe someone can prove me wrong
  else if ((!_ld_kernel) && (_ld_material))
  {
    mooseError("Lagrangian kernel small deformations with MOOSE material large"
               " deformations is not implemented");
  }
  // The easy case of small deformations: everything is just a copy!
  else
  {
    _cauchy_stress[_qp] = _stress_moose[_qp];
    _material_jacobian[_qp] = _jacobian_moose[_qp];
  }

  // Define the PK stress (for the homogenization system)
  if (_ld_kernel)
  {
    _PK1[_qp] = _J[_qp] * _cauchy_stress[_qp] * _F_inv[_qp].transpose();
  }
  else
  {
    _PK1[_qp] = _cauchy_stress[_qp];
  }
}

void
WrapStressLagrangianKernel::_objectiveUpdate()
{
  // All the common objective stress rates are going to need the
  // increment in the spatial velocity gradient and the increment in the
  // "unrotated" stress
  RankTwoTensor dS = _stress_moose[_qp] - _stress_moose_old[_qp];
  RankTwoTensor dL = RankTwoTensor::Identity() - _df[_qp];

  // We could plug in any objective update here but I'll just use Truesdell
  RankFourTensor J = _truesdellUpdate(dL);

  // Actually do the objective update to get the new Cauchy stress
  RankFourTensor Jinv = J.inverse();
  _cauchy_stress[_qp] = Jinv * (_cauchy_stress_old[_qp] + dS);

  // Again we could switch on the type of update but for now just
  // use Truesdell
  RankFourTensor U = _truesdellTangent(_cauchy_stress[_qp]);

  // Actually do the update of the tangent
  auto Isym = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
  _material_jacobian[_qp] = Jinv * (_jacobian_moose[_qp] * Isym - U);
}

RankFourTensor
WrapStressLagrangianKernel::_truesdellUpdate(const RankTwoTensor & dL)
{
  return _constructJ(dL);
}

RankFourTensor
WrapStressLagrangianKernel::_truesdellTangent(const RankTwoTensor & S)
{
  RankFourTensor U;
  auto I = RankTwoTensor::Identity();

  for (size_t m = 0; m < 3; m++)
  {
    for (size_t n = 0; n < 3; n++)
    {
      for (size_t k = 0; k < 3; k++)
      {
        for (size_t l = 0; l < 3; l++)
        {
          U(m, n, k, l) = I(k, l) * S(m, n) - I(m, k) * S(l, n) - I(n, k) * S(m, l);
        }
      }
    }
  }

  return U;
}

RankFourTensor
WrapStressLagrangianKernel::_constructJ(const RankTwoTensor & Q)
{
  RankFourTensor J;
  auto I = RankTwoTensor::Identity();
  auto trQ = Q.trace();

  for (size_t i = 0; i < 3; i++)
  {
    for (size_t j = 0; j < 3; j++)
    {
      for (size_t m = 0; m < 3; m++)
      {
        for (size_t n = 0; n < 3; n++)
        {
          J(i, j, m, n) = (1.0 + trQ) * I(i, m) * I(j, n) - Q(i, m) * I(j, n) - I(i, m) * Q(j, n);
        }
      }
    }
  }
  return J;
}
