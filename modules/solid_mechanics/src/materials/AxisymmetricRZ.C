/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AxisymmetricRZ.h"
#include "SolidModel.h"

#include "Problem.h"
#include "VolumetricModel.h"

namespace SolidMechanics
{

AxisymmetricRZ::AxisymmetricRZ(SolidModel & solid_model,
                               const std::string & name,
                               InputParameters parameters)
  :Element(solid_model, name, parameters),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _large_strain(solid_model.getParam<bool>("large_strain")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z"))
{
}

AxisymmetricRZ::~AxisymmetricRZ()
{
}

void
AxisymmetricRZ::computeStrain( const unsigned qp,
                               const SymmTensor & total_strain_old,
                               SymmTensor & total_strain_new,
                               SymmTensor & strain_increment )
{
  strain_increment.xx() = _grad_disp_r[qp](0);
  strain_increment.yy() = _grad_disp_z[qp](1);
  strain_increment.zz() = (_solid_model.q_point(qp)(0) != 0.0 ? _disp_r[qp]/_solid_model.q_point(qp)(0) : 0.0);
  strain_increment.xy() = 0.5*(_grad_disp_r[qp](1) + _grad_disp_z[qp](0));
  if (_large_strain)
  {
    strain_increment.xx() += 0.5*(_grad_disp_r[qp](0)*_grad_disp_r[qp](0) +
                                  _grad_disp_z[qp](0)*_grad_disp_z[qp](0));
    strain_increment.yy() += 0.5*(_grad_disp_r[qp](1)*_grad_disp_r[qp](1) +
                                  _grad_disp_z[qp](1)*_grad_disp_z[qp](1));
    strain_increment.zz() += 0.5*(strain_increment.zz()*strain_increment.zz());
    strain_increment.xy() += 0.5*(_grad_disp_r[qp](0)*_grad_disp_r[qp](1) +
                                  _grad_disp_z[qp](0)*_grad_disp_z[qp](1));
  }

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}


}
