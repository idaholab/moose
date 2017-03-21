/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PlaneStrain.h"
#include "SolidModel.h"
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

PlaneStrain::PlaneStrain(SolidModel & solid_model,
                         const std::string & name,
                         const InputParameters & parameters)
  : Element(solid_model, name, parameters),
    ScalarCoupleable(&solid_model),
    _large_strain(solid_model.getParam<bool>("large_strain")),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _have_strain_zz(isCoupled("strain_zz")),
    _strain_zz(_have_strain_zz ? coupledValue("strain_zz") : _zero),
    _have_scalar_strain_zz(isCoupledScalar("scalar_strain_zz")),
    _scalar_strain_zz(_have_scalar_strain_zz ? coupledScalarValue("scalar_strain_zz") : _zero),
    _volumetric_locking_correction(solid_model.getParam<bool>("volumetric_locking_correction"))
{
  if (_have_strain_zz && _have_scalar_strain_zz)
    mooseError("Must define only one of strain_zz or scalar_strain_zz");
}

PlaneStrain::~PlaneStrain() {}

void
PlaneStrain::computeStrain(const unsigned qp,
                           const SymmTensor & total_strain_old,
                           SymmTensor & total_strain_new,
                           SymmTensor & strain_increment)
{
  strain_increment.xx() = _grad_disp_x[qp](0);
  strain_increment.yy() = _grad_disp_y[qp](1);

  if (_have_strain_zz)
    strain_increment.zz() = _strain_zz[qp];
  else if (_have_scalar_strain_zz && _scalar_strain_zz.size() > 0)
    strain_increment.zz() = _scalar_strain_zz[0];
  else
    strain_increment.zz() = 0;

  strain_increment.xy() = 0.5 * (_grad_disp_x[qp](1) + _grad_disp_y[qp](0));
  strain_increment.yz() = 0;
  strain_increment.zx() = 0;
  if (_large_strain)
  {
    strain_increment.xx() += 0.5 * (_grad_disp_x[qp](0) * _grad_disp_x[qp](0) +
                                    _grad_disp_y[qp](0) * _grad_disp_y[qp](0));
    strain_increment.yy() += 0.5 * (_grad_disp_x[qp](1) * _grad_disp_x[qp](1) +
                                    _grad_disp_y[qp](1) * _grad_disp_y[qp](1));
    strain_increment.xy() += 0.5 * (_grad_disp_x[qp](0) * _grad_disp_x[qp](1) +
                                    _grad_disp_y[qp](0) * _grad_disp_y[qp](1));
  }

  if (_volumetric_locking_correction)
  {
    // volumetric locking correction
    Real volumetric_strain = 0.0;
    Real volume = 0.0;
    Real dim = 3.0;
    for (unsigned int qp_loop = 0; qp_loop < _solid_model.qrule()->n_points(); ++qp_loop)
    {
      if (_have_strain_zz)
        volumetric_strain +=
            (_grad_disp_x[qp_loop](0) + _grad_disp_y[qp_loop](1) + _strain_zz[qp_loop]) / dim *
            _solid_model.JxW(qp_loop);
      else if (_have_scalar_strain_zz && _scalar_strain_zz.size() > 0)
        volumetric_strain +=
            (_grad_disp_x[qp_loop](0) + _grad_disp_y[qp_loop](1) + _scalar_strain_zz[0]) / dim *
            _solid_model.JxW(qp_loop);
      else
        volumetric_strain +=
            (_grad_disp_x[qp_loop](0) + _grad_disp_y[qp_loop](1)) / dim * _solid_model.JxW(qp_loop);

      volume += _solid_model.JxW(qp_loop);

      if (_large_strain)
      {
        volumetric_strain += 0.5 * (_grad_disp_x[qp_loop](0) * _grad_disp_x[qp_loop](0) +
                                    _grad_disp_y[qp_loop](0) * _grad_disp_y[qp_loop](0)) /
                             dim * _solid_model.JxW(qp_loop);
        volumetric_strain += 0.5 * (_grad_disp_x[qp_loop](1) * _grad_disp_x[qp_loop](1) +
                                    _grad_disp_y[qp_loop](1) * _grad_disp_y[qp_loop](1)) /
                             dim * _solid_model.JxW(qp_loop);
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

void
PlaneStrain::computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0, 0) = _grad_disp_x[qp](0) + 1.0;
  F(0, 1) = _grad_disp_x[qp](1);
  F(0, 2) = 0.0;
  F(1, 0) = _grad_disp_y[qp](0);
  F(1, 1) = _grad_disp_y[qp](1) + 1.0;
  F(1, 2) = 0.0;
  F(2, 0) = 0.0;
  F(2, 1) = 0.0;
  F(2, 2) = 1.0;
}
}
