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
  t->setYoungsModulus(_youngs_modulus);
  t->setPoissonsRatio(_poissons_ratio);

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

  SymmTensor elastic_strain;
  computeNetElasticStrain(_strain_increment, elastic_strain);

  // C * e
  _stress[_qp] = _elasticity_tensor[_qp] * elastic_strain;
  _stress[_qp] += _stress_old;

}

