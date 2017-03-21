/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NonlinearPlaneStrain.h"
#include "SolidModel.h"
#include "Problem.h"
#include "SymmIsotropicElasticityTensor.h"

// libmesh includes
#include "libmesh/quadrature.h"

namespace SolidMechanics
{

NonlinearPlaneStrain::NonlinearPlaneStrain(SolidModel & solid_model,
                                           const std::string & name,
                                           const InputParameters & parameters)
  : Nonlinear(solid_model, name, parameters),
    ScalarCoupleable(&solid_model),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _have_strain_zz(isCoupled("strain_zz")),
    _strain_zz(_have_strain_zz ? coupledValue("strain_zz") : _zero),
    _have_scalar_strain_zz(isCoupledScalar("scalar_strain_zz")),
    _scalar_strain_zz(_have_scalar_strain_zz ? coupledScalarValue("scalar_strain_zz") : _zero),
    _grad_disp_x_old(coupledGradientOld("disp_x")),
    _grad_disp_y_old(coupledGradientOld("disp_y")),
    _strain_zz_old(_have_strain_zz ? coupledValueOld("strain_zz") : _zero),
    _scalar_strain_zz_old(_have_scalar_strain_zz ? coupledScalarValueOld("scalar_strain_zz")
                                                 : _zero),
    _volumetric_locking_correction(_solid_model.getParam<bool>("volumetric_locking_correction"))
{
}

////////////////////////////////////////////////////////////////////////

NonlinearPlaneStrain::~NonlinearPlaneStrain() {}

////////////////////////////////////////////////////////////////////////

void
NonlinearPlaneStrain::computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat)
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
    Real strain_zz, strain_zz_old;
    if (_have_strain_zz)
    {
      strain_zz = _strain_zz[qp];
      strain_zz_old = _strain_zz_old[qp];
    }
    else if (_have_scalar_strain_zz && _scalar_strain_zz.size() > 0)
    {
      strain_zz = _scalar_strain_zz[qp];
      strain_zz_old = _scalar_strain_zz_old[qp];
    }
    else
    {
      strain_zz = 0.0;
      strain_zz_old = 0.0;
    }

    fillMatrix(qp, _grad_disp_x, _grad_disp_y, strain_zz, A);
    fillMatrix(qp, _grad_disp_x_old, _grad_disp_y_old, strain_zz_old, Fbar);

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
NonlinearPlaneStrain::computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0, 0) = _grad_disp_x[qp](0) + 1;
  F(0, 1) = _grad_disp_x[qp](1);
  F(0, 2) = 0;
  F(1, 0) = _grad_disp_y[qp](0);
  F(1, 1) = _grad_disp_y[qp](1) + 1;
  F(1, 2) = 0;
  F(2, 0) = 0;
  F(2, 1) = 0;
  if (_have_strain_zz)
    F(2, 2) = _strain_zz[qp] + 1;
  else if (_have_scalar_strain_zz && _scalar_strain_zz.size() > 0)
    F(2, 2) = _scalar_strain_zz[qp] + 1;
  else
    F(2, 2) = 1;
}

////////////////////////////////////////////////////////////////////////

void
NonlinearPlaneStrain::fillMatrix(unsigned int qp,
                                 const VariableGradient & grad_x,
                                 const VariableGradient & grad_y,
                                 const Real & strain_zz,
                                 ColumnMajorMatrix & A) const
{
  mooseAssert(A.n() == 3 && A.m() == 3, "computeDefGrad requires 3x3 matrix");

  A(0, 0) = grad_x[qp](0);
  A(0, 1) = grad_x[qp](1);
  A(0, 2) = 0;
  A(1, 0) = grad_y[qp](0);
  A(1, 1) = grad_y[qp](1);
  A(1, 2) = 0;
  A(2, 0) = 0;
  A(2, 1) = 0;
  A(2, 2) = strain_zz;
}

//////////////////////////////////////////////////////////////////////////

Real
NonlinearPlaneStrain::volumeRatioOld(unsigned int qp) const
{
  Real strain_zz_old;
  if (_have_strain_zz)
    strain_zz_old = _strain_zz_old[qp];
  else if (_have_scalar_strain_zz && _scalar_strain_zz.size() > 0)
    strain_zz_old = _scalar_strain_zz_old[qp];
  else
    strain_zz_old = 0.0;

  ColumnMajorMatrix Fnm1T;
  fillMatrix(qp, _grad_disp_x_old, _grad_disp_y_old, strain_zz_old, Fnm1T);
  Fnm1T.addDiag(1);

  return detMatrix(Fnm1T);
}

//////////////////////////////////////////////////////////////////////////
}
