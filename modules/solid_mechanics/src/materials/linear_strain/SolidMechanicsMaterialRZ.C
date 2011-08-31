#include "SolidMechanicsMaterialRZ.h"

#include "SymmIsotropicElasticityTensorRZ.h"
#include "MaterialModel.h"
#include "Problem.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>()
{
  InputParameters params = validParams<SolidModel>();
  params.addRequiredCoupledVar("disp_r", "The r displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addParam<bool>("large_strain", false, "Whether to include large strain terms");
  return params;
}

SolidMechanicsMaterialRZ::SolidMechanicsMaterialRZ(const std::string & name,
                                                   InputParameters parameters)
  :SolidModel(name, parameters),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _large_strain(getParam<bool>("large_strain")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z"))
{
  SymmIsotropicElasticityTensorRZ * t = new SymmIsotropicElasticityTensorRZ;
  mooseAssert(_lambda_set, "Internal error:  lambda not set");
  t->setLambda(_lambda);
  mooseAssert(_shear_modulus_set, "Internal error:  shear modulus not set");
  t->setShearModulus(_shear_modulus);
  t->calculate(0);

  elasticityTensor( t );
}

SolidMechanicsMaterialRZ::~SolidMechanicsMaterialRZ()
{
}

void
SolidMechanicsMaterialRZ::computeStrain()
{
  _strain_increment.xx() = _grad_disp_r[_qp](0);
  _strain_increment.yy() = _grad_disp_z[_qp](1);
  _strain_increment.zz() = _disp_r[_qp]/_q_point[_qp](0);
  _strain_increment.xy() = 0.5*(_grad_disp_r[_qp](1) + _grad_disp_z[_qp](0));
  if (_large_strain)
  {
    _strain_increment.xx() += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](0) +
                                   _grad_disp_z[_qp](0)*_grad_disp_z[_qp](0));
    _strain_increment.yy() += 0.5*(_grad_disp_r[_qp](1)*_grad_disp_r[_qp](1) +
                                   _grad_disp_z[_qp](1)*_grad_disp_z[_qp](1));
    _strain_increment.zz() += 0.5*(_strain_increment.zz()*_strain_increment.zz());
    _strain_increment.xy() += 0.5*(_grad_disp_r[_qp](0)*_grad_disp_r[_qp](1) +
                                   _grad_disp_z[_qp](0)*_grad_disp_z[_qp](1));
  }

  _total_strain[_qp] = _strain_increment;

  _strain_increment -= _total_strain_old[_qp];
}

void
SolidMechanicsMaterialRZ::computeStress()
{

  const SymmTensor input_strain(_strain_increment);
  computeNetElasticStrain(input_strain, _strain_increment);

  // C * e
  _stress[_qp] = _elasticity_tensor[_qp] * _strain_increment;
  _stress[_qp] += _stress_old;

}

unsigned int
SolidMechanicsMaterialRZ::getNumKnownCrackDirs() const
{
  unsigned int retVal(1);
  for (unsigned int i(0); i < 2; ++i)
  {
   retVal += ((*_crack_flags_old)[_qp](i) < 1);
  }
  return retVal;
}
