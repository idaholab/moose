/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PlaneStrain.h"
#include "SolidModel.h"

namespace SolidMechanics
{

PlaneStrain::PlaneStrain(SolidModel & solid_model,
                         const std::string & name,
                         const InputParameters & parameters)
  :Element(solid_model, name, parameters),
   ScalarCoupleable(&solid_model),
   _large_strain(solid_model.getParam<bool>("large_strain")),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _have_strain_zz(isCoupled("strain_zz")),
   _strain_zz(_have_strain_zz?coupledValue("strain_zz"):_zero),
   _have_scalar_strain_zz(isCoupledScalar("scalar_strain_zz")),
   _scalar_strain_zz(_have_scalar_strain_zz?coupledScalarValue("scalar_strain_zz"):_zero)
{
  if (_have_strain_zz && _have_scalar_strain_zz)
      mooseError("Must define only one of strain_zz or scalar_strain_zz");
}

PlaneStrain::~PlaneStrain()
{
}

void
PlaneStrain::computeStrain( const unsigned qp,
                               const SymmTensor & total_strain_old,
                               SymmTensor & total_strain_new,
                               SymmTensor & strain_increment )
{
  strain_increment.xx() = _grad_disp_x[qp](0);
  strain_increment.yy() = _grad_disp_y[qp](1);

  if (_have_strain_zz)
    strain_increment.zz() = _strain_zz[qp];
  else if (_have_scalar_strain_zz && _scalar_strain_zz.size()>0)
    strain_increment.zz() = _scalar_strain_zz[0];
  else
    strain_increment.zz() = 0;

  strain_increment.xy() = 0.5*(_grad_disp_x[qp](1) + _grad_disp_y[qp](0));
  strain_increment.yz() = 0;
  strain_increment.zx() = 0;
  if (_large_strain)
  {
    strain_increment.xx() += 0.5*(_grad_disp_x[qp](0)*_grad_disp_x[qp](0) +
                                  _grad_disp_y[qp](0)*_grad_disp_y[qp](0));
    strain_increment.yy() += 0.5*(_grad_disp_x[qp](1)*_grad_disp_x[qp](1) +
                                  _grad_disp_y[qp](1)*_grad_disp_y[qp](1));
    strain_increment.xy() += 0.5*(_grad_disp_x[qp](0)*_grad_disp_x[qp](1) +
                                  _grad_disp_y[qp](0)*_grad_disp_y[qp](1));
  }

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}

void
PlaneStrain::computeDeformationGradient( unsigned int qp, ColumnMajorMatrix & F)
{
  mooseAssert(F.n() == 3 && F.m() == 3, "computeDefGrad requires 3x3 matrix");

  F(0,0) = _grad_disp_x[qp](0) + 1.0;
  F(0,1) = _grad_disp_x[qp](1);
  F(0,2) = 0.0;
  F(1,0) = _grad_disp_y[qp](0);
  F(1,1) = _grad_disp_y[qp](1) + 1.0;
  F(1,2) = 0.0;
  F(2,0) = 0.0;
  F(2,1) = 0.0;
  F(2,2) = 1.0;
}



}
