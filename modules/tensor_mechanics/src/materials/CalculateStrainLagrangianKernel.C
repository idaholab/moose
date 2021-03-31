#include "CalculateStrainLagrangianKernel.h"

registerMooseObject("TensorMechanicsApp", CalculateStrainLagrangianKernel);

InputParameters
CalculateStrainLagrangianKernel::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("displacements", "Displacement variables");
  params.addParam<bool>(
      "kernel_large_kinematics", false, "Use large displacement kinematics in the kernel.");
  params.addParam<bool>("material_large_kinematics",
                        false,
                        "If true the material returns the Cauchy stress"
                        " and the derivative with respect to the spatial "
                        "velocity gradient directly.");
  params.addParam<std::vector<MaterialPropertyName>>("eigenstrain_names",
                                                     "List of eigenstrains to account for.");
  params.addCoupledVar("macro_gradient", "Optional scalar field with the macro gradient");

  // We rely on this *not* having use_displaced mesh on
  params.suppressParameter<bool>("use_displaced_mesh");

  return params;
}

CalculateStrainLagrangianKernel::CalculateStrainLagrangianKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _grad_disp(3),
    _ld_kernel(getParam<bool>("kernel_large_kinematics")),
    _ld_material(getParam<bool>("material_large_kinematics")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _eigenstrains_old(_eigenstrain_names.size()),
    _total_strain(declareProperty<RankTwoTensor>("total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>("total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>("mechanical_strain")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>("mechanical_strain")),
    _strain_increment(declareProperty<RankTwoTensor>("strain_increment")),
    _rotation_increment(declareProperty<RankTwoTensor>("rotation_increment")),
    _def_grad(declareProperty<RankTwoTensor>("deformation_gradient")),
    _def_grad_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _df(declareProperty<RankTwoTensor>("inc_def_grad")),
    _inv_def_grad(declareProperty<RankTwoTensor>("inv_def_grad")),
    _detJ(declareProperty<Real>("detJ")),
    _macro_gradient(isCoupledScalar("macro_gradient", 0) ? coupledScalarValue("macro_gradient")
                                                         : _zero),
    _homogenization_contribution(declareProperty<RankTwoTensor>("homogenization_contribution"))
{
  // Setup eigenstrains
  for (unsigned int i = 0; i < _eigenstrain_names.size(); i++)
  {
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
  }
}

void
CalculateStrainLagrangianKernel::initialSetup()
{
  // Grab the actual displacements
  for (unsigned int i = 0; i < _ndisp; i++)
  {
    _disp[i] = &coupledValue("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
  }

  // All others zero (so this will work naturally for plane strain problems)
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    _disp[i] = &_zero;
    _grad_disp[i] = &_grad_zero;
  }
}

void
CalculateStrainLagrangianKernel::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
  _mechanical_strain[_qp].zero();
  _strain_increment[_qp].zero();
  _rotation_increment[_qp] = RankTwoTensor::Identity();
  _def_grad[_qp] = RankTwoTensor::Identity();
  _df[_qp] = RankTwoTensor::Identity();
  _inv_def_grad[_qp] = RankTwoTensor::Identity();
  _detJ[_qp] = 1.0;
  _homogenization_contribution[_qp].zero();
}

void
CalculateStrainLagrangianKernel::computeQpProperties()
{
  // We'll calculate the deformation gradient even for small kinematics
  _def_grad[_qp] =
      (RankTwoTensor::Identity() +
       RankTwoTensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]));

  // Add in the macorscale gradient contribution
  _homogenization_contribution[_qp] = _homogenizationContribution();
  _def_grad[_qp] += _homogenization_contribution[_qp];

  // If the kernel is large deformation then we need the "actual"
  // kinematic quantities
  RankTwoTensor L;
  if (_ld_kernel)
  {
    _inv_def_grad[_qp] = _def_grad[_qp].inverse();
    _detJ[_qp] = _def_grad[_qp].det();
    _df[_qp] = _def_grad_old[_qp] * _inv_def_grad[_qp];
    L = RankTwoTensor::Identity() - _df[_qp];
  }
  // For small deformations we just provide the identity
  else
  {
    _inv_def_grad[_qp] = RankTwoTensor::Identity();
    _detJ[_qp] = 1.0;
    _df[_qp] = RankTwoTensor::Identity();
    L = _def_grad[_qp] - _def_grad_old[_qp];
  }

  // This is a useful case, but I need to figure out what these MOOSE
  // models need and return
  if (_ld_kernel && _ld_material)
  {
    mooseError("Strain calculations for material large displacements not"
               " implemented");
  }
  // This is for the objective update
  else if (_ld_kernel && !_ld_material)
  {
    // Supply the deformation rate as the increment in strain
    _calculateIncrementalStrains(L);
  }
  // This case makes no sense
  else if (!_ld_kernel && _ld_material)
  {
    mooseError("Lagrangian kernel small deformations with MOOSE material large"
               " deformations is not implemented");
  }
  // Small strains
  else
  {
    // Need total_strain and mechanical_strain as small strains
    _calculateIncrementalStrains(L);
  }
}

void
CalculateStrainLagrangianKernel::_calculateIncrementalStrains(RankTwoTensor L)
{
  // Get the deformation increments
  RankTwoTensor D = (L + L.transpose()) / 2.0;
  _strain_increment[_qp] = D - _eigenstrainIncrement();

  // Don't do it MOOSE! The WrapStress object will take care of it
  _rotation_increment[_qp] = RankTwoTensor::Identity();

  // Increment the total strain
  _total_strain[_qp] = _total_strain_old[_qp] + D;

  // Increment the mechanical strain
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
}

RankTwoTensor
CalculateStrainLagrangianKernel::_eigenstrainIncrement()
{
  // Sum the eigenstrains
  RankTwoTensor res;
  res.zero();
  for (size_t i = 0; i < _eigenstrain_names.size(); i++)
  {
    res += ((*_eigenstrains[i])[_qp] - (*_eigenstrains_old[i])[_qp]);
  }
  return res;
}

RankTwoTensor
CalculateStrainLagrangianKernel::_homogenizationContribution()
{
  if (isCoupledScalar("macro_gradient", 0))
  {
    if (_ld_kernel)
    {
      if (_ndisp == 1)
      {
        return RankTwoTensor(_macro_gradient[0], 0, 0, 0, 0, 0, 0, 0, 0);
      }
      else if (_ndisp == 2)
      {
        return RankTwoTensor(_macro_gradient[0],
                             _macro_gradient[2],
                             0,
                             _macro_gradient[3],
                             _macro_gradient[1],
                             0,
                             0,
                             0,
                             0);
      }
      else
      {
        return RankTwoTensor(_macro_gradient[0],
                             _macro_gradient[1],
                             _macro_gradient[2],
                             _macro_gradient[3],
                             _macro_gradient[4],
                             _macro_gradient[5],
                             _macro_gradient[6],
                             _macro_gradient[7],
                             _macro_gradient[8]);
      }
    }
    else
    {
      if (_ndisp == 1)
      {
        return RankTwoTensor(_macro_gradient[0], 0, 0, 0, 0, 0, 0, 0, 0);
      }
      else if (_ndisp == 2)
      {
        return RankTwoTensor(
            _macro_gradient[0], 0, 0, _macro_gradient[2], _macro_gradient[1], 0, 0, 0, 0);
      }
      else
      {
        return RankTwoTensor(_macro_gradient[0],
                             0.0,
                             0.0,
                             _macro_gradient[5],
                             _macro_gradient[1],
                             0.0,
                             _macro_gradient[4],
                             _macro_gradient[3],
                             _macro_gradient[2]);
      }
    }
  }
  else
  {
    RankTwoTensor res;
    res.zero();
    return res;
  }
}
