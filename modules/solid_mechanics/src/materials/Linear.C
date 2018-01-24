/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Linear.h"
#include "SolidModel.h"
#include "Problem.h"
#include "MooseMesh.h"
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

Linear::Linear(SolidModel & solid_model,
               const std::string & name,
               const InputParameters & parameters)
  : Element(solid_model, name, parameters),
    _large_strain(solid_model.getParam<bool>("large_strain")),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(parameters.get<SubProblem *>("_subproblem")->mesh().dimension() == 3
                     ? coupledGradient("disp_z")
                     : _grad_zero),
    _volumetric_locking_correction(solid_model.getParam<bool>("volumetric_locking_correction"))
{
}

Linear::~Linear() {}

void
Linear::computeStrain(const unsigned qp,
                      const SymmTensor & total_strain_old,
                      SymmTensor & total_strain_new,
                      SymmTensor & strain_increment)
{
  strain_increment.xx(_grad_disp_x[qp](0));
  strain_increment.yy(_grad_disp_y[qp](1));
  strain_increment.zz(_grad_disp_z[qp](2));
  strain_increment.xy(0.5 * (_grad_disp_x[qp](1) + _grad_disp_y[qp](0)));
  strain_increment.yz(0.5 * (_grad_disp_y[qp](2) + _grad_disp_z[qp](1)));
  strain_increment.zx(0.5 * (_grad_disp_z[qp](0) + _grad_disp_x[qp](2)));
  if (_large_strain)
  {
    strain_increment.xx() += 0.5 * (_grad_disp_x[qp](0) * _grad_disp_x[qp](0) +
                                    _grad_disp_y[qp](0) * _grad_disp_y[qp](0) +
                                    _grad_disp_z[qp](0) * _grad_disp_z[qp](0));
    strain_increment.yy() += 0.5 * (_grad_disp_x[qp](1) * _grad_disp_x[qp](1) +
                                    _grad_disp_y[qp](1) * _grad_disp_y[qp](1) +
                                    _grad_disp_z[qp](1) * _grad_disp_z[qp](1));
    strain_increment.zz() += 0.5 * (_grad_disp_x[qp](2) * _grad_disp_x[qp](2) +
                                    _grad_disp_y[qp](2) * _grad_disp_y[qp](2) +
                                    _grad_disp_z[qp](2) * _grad_disp_z[qp](2));
    strain_increment.xy() += 0.5 * (_grad_disp_x[qp](0) * _grad_disp_x[qp](1) +
                                    _grad_disp_y[qp](0) * _grad_disp_y[qp](1) +
                                    _grad_disp_z[qp](0) * _grad_disp_z[qp](1));
    strain_increment.yz() += 0.5 * (_grad_disp_x[qp](1) * _grad_disp_x[qp](2) +
                                    _grad_disp_y[qp](1) * _grad_disp_y[qp](2) +
                                    _grad_disp_z[qp](1) * _grad_disp_z[qp](2));
    strain_increment.zx() += 0.5 * (_grad_disp_x[qp](2) * _grad_disp_x[qp](0) +
                                    _grad_disp_y[qp](2) * _grad_disp_y[qp](0) +
                                    _grad_disp_z[qp](2) * _grad_disp_z[qp](0));
  }

  if (_volumetric_locking_correction)
  {
    // volumetric locking correction - averaging the volumertic strain over the element
    Real volumetric_strain = 0.0;
    Real volume = 0.0;
    for (unsigned int qp_loop = 0; qp_loop < _solid_model.qrule()->n_points(); ++qp_loop)
    {
      volumetric_strain +=
          (_grad_disp_x[qp_loop](0) + _grad_disp_y[qp_loop](1) + _grad_disp_z[qp_loop](2)) / 3.0 *
          _solid_model.JxW(qp_loop);

      volume += _solid_model.JxW(qp_loop);

      if (_large_strain)
      {
        volumetric_strain += 0.5 * (_grad_disp_x[qp_loop](0) * _grad_disp_x[qp_loop](0) +
                                    _grad_disp_y[qp_loop](0) * _grad_disp_y[qp_loop](0) +
                                    _grad_disp_z[qp_loop](0) * _grad_disp_z[qp_loop](0)) /
                             3.0 * _solid_model.JxW(qp_loop);
        volumetric_strain += 0.5 * (_grad_disp_x[qp_loop](1) * _grad_disp_x[qp_loop](1) +
                                    _grad_disp_y[qp_loop](1) * _grad_disp_y[qp_loop](1) +
                                    _grad_disp_z[qp_loop](1) * _grad_disp_z[qp_loop](1)) /
                             3.0 * _solid_model.JxW(qp_loop);
        volumetric_strain += 0.5 * (_grad_disp_x[qp_loop](2) * _grad_disp_x[qp_loop](2) +
                                    _grad_disp_y[qp_loop](2) * _grad_disp_y[qp_loop](2) +
                                    _grad_disp_z[qp_loop](2) * _grad_disp_z[qp_loop](2)) /
                             3.0 * _solid_model.JxW(qp_loop);
      }
    }

    volumetric_strain /= volume; // average volumetric strain

    // strain increment at _qp
    Real trace = strain_increment.trace();
    strain_increment.xx() += volumetric_strain - trace / 3.0;
    strain_increment.yy() += volumetric_strain - trace / 3.0;
    strain_increment.zz() += volumetric_strain - trace / 3.0;
  }

  total_strain_new = strain_increment;
  strain_increment -= total_strain_old;
}
}
