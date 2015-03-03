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
                         InputParameters parameters)
  :Element(solid_model, name, parameters),
   _large_strain(solid_model.getParam<bool>("large_strain")),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y"))
{
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
