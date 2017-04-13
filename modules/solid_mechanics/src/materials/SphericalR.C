/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SphericalR.h"

#include "Problem.h"
#include "SolidModel.h"

namespace SolidMechanics
{

SphericalR::SphericalR(SolidModel & solid_model,
                       const std::string & name,
                       const InputParameters & parameters)
  : Element(solid_model, name, parameters),
    _disp_r(coupledValue("disp_r")),
    _large_strain(solid_model.getParam<bool>("large_strain")),
    _grad_disp_r(coupledGradient("disp_r"))
{
}

SphericalR::~SphericalR() {}

void
SphericalR::computeStrain(const unsigned qp,
                          const SymmTensor & total_strain_old,
                          SymmTensor & total_strain_new,
                          SymmTensor & strain_increment)
{
  strain_increment.xx() = _grad_disp_r[qp](0);
  strain_increment.yy() =
      (_solid_model.q_point(qp)(0) != 0.0 ? _disp_r[qp] / _solid_model.q_point(qp)(0) : 0.0);
  strain_increment.zz() = strain_increment.yy();
  if (_large_strain)
  {
    strain_increment.xx() += 0.5 * (_grad_disp_r[qp](0) * _grad_disp_r[qp](0));
    strain_increment.yy() += 0.5 * (strain_increment.yy() * strain_increment.yy());
    strain_increment.zz() += 0.5 * (strain_increment.zz() * strain_increment.zz());
  }

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}
}
