/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NonlinearRZ.h"
#include "SolidModel.h"
#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"

// libmesh includes
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

NonlinearRZ::NonlinearRZ(SolidModel & solid_model,
                         const std::string & name,
                         const InputParameters & parameters)
  : Nonlinear(solid_model, name, parameters),
    _grad_disp_r(coupledGradient("disp_r")),
    _grad_disp_z(coupledGradient("disp_z")),
    _grad_disp_r_old(coupledGradientOld("disp_r")),
    _grad_disp_z_old(coupledGradientOld("disp_z")),
    _disp_r(coupledValue("disp_r")),
    _disp_r_old(coupledValueOld("disp_r")),
    _volumetric_locking_correction(_solid_model.getParam<bool>("volumetric_locking_correction"))
{
}

////////////////////////////////////////////////////////////////////////

NonlinearRZ::~NonlinearRZ() {}

////////////////////////////////////////////////////////////////////////

void
NonlinearRZ::computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat)
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
    fillMatrix(qp, _grad_disp_r, _grad_disp_z, _disp_r, A);
    fillMatrix(qp, _grad_disp_r_old, _grad_disp_z_old, _disp_r_old, Fbar);

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
      Fhat_average += Fhat[qp] * _solid_model.JxW(qp) * _solid_model.q_point(qp)(0);

      volume += _solid_model.JxW(qp) *
                _solid_model.q_point(qp)(0); // Accumulate original configuration volume
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
NonlinearRZ::computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0, 0) = _grad_disp_r[qp](0) + 1;
  F(0, 1) = _grad_disp_r[qp](1);
  F(0, 2) = 0;
  F(1, 0) = _grad_disp_z[qp](0);
  F(1, 1) = _grad_disp_z[qp](1) + 1;
  F(1, 2) = 0;
  F(2, 0) = 0;
  F(2, 1) = 0;
  F(2, 2) =
      (_solid_model.q_point(qp)(0) != 0.0 ? _disp_r[qp] / _solid_model.q_point(qp)(0) : 0.0) + 1;
}

////////////////////////////////////////////////////////////////////////

void
NonlinearRZ::fillMatrix(unsigned int qp,
                        const VariableGradient & grad_r,
                        const VariableGradient & grad_z,
                        const VariableValue & u,
                        ColumnMajorMatrix & A) const
{
  mooseAssert(A.n() == 3 && A.m() == 3, "computeDefGrad requires 3x3 matrix");

  A(0, 0) = grad_r[qp](0);
  A(0, 1) = grad_r[qp](1);
  A(0, 2) = 0;
  A(1, 0) = grad_z[qp](0);
  A(1, 1) = grad_z[qp](1);
  A(1, 2) = 0;
  A(2, 0) = 0;
  A(2, 1) = 0;
  A(2, 2) = (_solid_model.q_point(qp)(0) != 0.0 ? u[qp] / _solid_model.q_point(qp)(0) : 0.0);
}

//////////////////////////////////////////////////////////////////////////

Real
NonlinearRZ::volumeRatioOld(unsigned int qp) const
{
  ColumnMajorMatrix Fnm1T;
  fillMatrix(qp, _grad_disp_r_old, _grad_disp_z_old, _disp_r_old, Fnm1T);
  Fnm1T.addDiag(1);

  return detMatrix(Fnm1T);
}

//////////////////////////////////////////////////////////////////////////
}
