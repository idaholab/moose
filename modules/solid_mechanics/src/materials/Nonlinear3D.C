/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Nonlinear3D.h"
#include "SolidModel.h"
#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"

// libmesh includes
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

Nonlinear3D::Nonlinear3D(SolidModel & solid_model,
                         const std::string & name,
                         const InputParameters & parameters)
  : Nonlinear(solid_model, name, parameters),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(coupledGradient("disp_z")),
    _grad_disp_x_old(coupledGradientOld("disp_x")),
    _grad_disp_y_old(coupledGradientOld("disp_y")),
    _grad_disp_z_old(coupledGradientOld("disp_z")),
    _volumetric_locking_correction(_solid_model.getParam<bool>("volumetric_locking_correction"))
{
}

////////////////////////////////////////////////////////////////////////

Nonlinear3D::~Nonlinear3D() {}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat)
{
  // A = grad(u(k+1) - u(k))
  // Fbar = 1 + grad(u(k))
  // Fhat = 1 + A*(Fbar^-1)
  ColumnMajorMatrix A;
  ColumnMajorMatrix Fbar;
  ColumnMajorMatrix Fbar_inverse;
  ColumnMajorMatrix Fhat_average;
  Real volume(0);

  _Fbar.resize(_solid_model.qrule()->n_points());

  for (unsigned qp = 0; qp < _solid_model.qrule()->n_points(); ++qp)
  {
    fillMatrix(qp, _grad_disp_x, _grad_disp_y, _grad_disp_z, A);
    fillMatrix(qp, _grad_disp_x_old, _grad_disp_y_old, _grad_disp_z_old, Fbar);

    A -= Fbar;

    Fbar.addDiag(1);

    _Fbar[qp] = Fbar;

    // Get Fbar^(-1)
    // Computing the inverse is generally a bad idea.
    // It's better to compute LU factors.   For now at least, we'll take
    // a direct route.

    invertMatrix(Fbar, Fbar_inverse);

    Fhat[qp] = A * Fbar_inverse;
    Fhat[qp].addDiag(1);

    if (_volumetric_locking_correction)
    {
      // Now include the contribution for the integration of Fhat over the element
      Fhat_average += Fhat[qp] * _solid_model.JxW(qp);

      volume += _solid_model.JxW(qp); // Accumulate original configuration volume
    }
  }

  if (_volumetric_locking_correction)
  {
    Fhat_average /= volume;
    const Real det_Fhat_average(detMatrix(Fhat_average));

    // Finalize volumetric locking correction
    for (unsigned qp = 0; qp < _solid_model.qrule()->n_points(); ++qp)
    {
      const Real det_Fhat(detMatrix(Fhat[qp]));
      const Real factor(std::cbrt(det_Fhat_average / det_Fhat));

      Fhat[qp] *= factor;
    }
  }
  //    Moose::out << "Fhat(0,0)" << Fhat[0](0,0) << std::endl;
}

////////////////////////////////////////////////////////////////////////

void
Nonlinear3D::computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0, 0) = _grad_disp_x[qp](0) + 1;
  F(0, 1) = _grad_disp_x[qp](1);
  F(0, 2) = _grad_disp_x[qp](2);
  F(1, 0) = _grad_disp_y[qp](0);
  F(1, 1) = _grad_disp_y[qp](1) + 1;
  F(1, 2) = _grad_disp_y[qp](2);
  F(2, 0) = _grad_disp_z[qp](0);
  F(2, 1) = _grad_disp_z[qp](1);
  F(2, 2) = _grad_disp_z[qp](2) + 1;
}

//////////////////////////////////////////////////////////////////////////

Real
Nonlinear3D::volumeRatioOld(unsigned int qp) const
{
  ColumnMajorMatrix Fnm1T(_grad_disp_x_old[qp], _grad_disp_y_old[qp], _grad_disp_z_old[qp]);
  Fnm1T(0, 0) += 1;
  Fnm1T(1, 1) += 1;
  Fnm1T(2, 2) += 1;

  return detMatrix(Fnm1T);
}

////////////////////////////////////////////////////////////////////////
}
