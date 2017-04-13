/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AxisymmetricRZ.h"
#include "SolidModel.h"

#include "Problem.h"
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

AxisymmetricRZ::AxisymmetricRZ(SolidModel & solid_model,
                               const std::string & name,
                               const InputParameters & parameters)
  : Element(solid_model, name, parameters),
    _disp_r(coupledValue("disp_r")),
    _disp_z(coupledValue("disp_z")),
    _large_strain(solid_model.getParam<bool>("large_strain")),
    _grad_disp_r(coupledGradient("disp_r")),
    _grad_disp_z(coupledGradient("disp_z")),
    _volumetric_locking_correction(solid_model.getParam<bool>("volumetric_locking_correction"))
{
}

AxisymmetricRZ::~AxisymmetricRZ() {}

void
AxisymmetricRZ::computeStrain(const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment)
{
  strain_increment.xx() = _grad_disp_r[qp](0);
  strain_increment.yy() = _grad_disp_z[qp](1);
  strain_increment.zz() =
      (_solid_model.q_point(qp)(0) != 0.0 ? _disp_r[qp] / _solid_model.q_point(qp)(0) : 0.0);
  strain_increment.xy() = 0.5 * (_grad_disp_r[qp](1) + _grad_disp_z[qp](0));
  strain_increment.yz() = 0;
  strain_increment.zx() = 0;
  if (_large_strain)
  {
    strain_increment.xx() += 0.5 * (_grad_disp_r[qp](0) * _grad_disp_r[qp](0) +
                                    _grad_disp_z[qp](0) * _grad_disp_z[qp](0));
    strain_increment.yy() += 0.5 * (_grad_disp_r[qp](1) * _grad_disp_r[qp](1) +
                                    _grad_disp_z[qp](1) * _grad_disp_z[qp](1));
    strain_increment.zz() += 0.5 * (strain_increment.zz() * strain_increment.zz());
    strain_increment.xy() += 0.5 * (_grad_disp_r[qp](0) * _grad_disp_r[qp](1) +
                                    _grad_disp_z[qp](0) * _grad_disp_z[qp](1));
  }

  if (_volumetric_locking_correction)
  {
    // volumetric locking correction
    Real volumetric_strain = 0.0;
    Real volume = 0.0;
    Real dim = 3.0;
    for (unsigned int qp_loop = 0; qp_loop < _solid_model.qrule()->n_points(); ++qp_loop)
    {
      if (_solid_model.q_point(qp)(0) != 0.0)
        volumetric_strain += (_grad_disp_r[qp_loop](0) + _grad_disp_z[qp_loop](1) +
                              _disp_r[qp_loop] / _solid_model.q_point(qp_loop)(0)) /
                             dim * _solid_model.JxW(qp_loop) * _solid_model.q_point(qp_loop)(0);
      else
        volumetric_strain += (_grad_disp_r[qp_loop](0) + _grad_disp_z[qp_loop](1)) / dim *
                             _solid_model.JxW(qp_loop) * _solid_model.q_point(qp_loop)(0);

      volume += _solid_model.JxW(qp_loop) * _solid_model.q_point(qp_loop)(0);

      if (_large_strain)
      {
        volumetric_strain += 0.5 * (_grad_disp_r[qp_loop](0) * _grad_disp_r[qp_loop](0) +
                                    _grad_disp_z[qp_loop](0) * _grad_disp_z[qp_loop](0)) /
                             dim * _solid_model.JxW(qp_loop) * _solid_model.q_point(qp_loop)(0);
        volumetric_strain += 0.5 * (_grad_disp_r[qp_loop](1) * _grad_disp_r[qp_loop](1) +
                                    _grad_disp_z[qp_loop](1) * _grad_disp_z[qp_loop](1)) /
                             dim * _solid_model.JxW(qp_loop) * _solid_model.q_point(qp_loop)(0);
      }
    }

    volumetric_strain /= volume; // average volumetric strain

    // strain increment at _qp
    Real trace = strain_increment.trace();
    strain_increment.xx() += volumetric_strain - trace / dim;
    strain_increment.yy() += volumetric_strain - trace / dim;
    strain_increment.zz() += volumetric_strain - trace / dim;
  }

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}
}
