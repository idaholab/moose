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

namespace SolidMechanics
{

Linear::Linear(SolidModel & solid_model,
               const std::string & name,
               const InputParameters & parameters)
  :Element(solid_model, name, parameters),
   _large_strain(solid_model.getParam<bool>("large_strain")),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(parameters.get<SubProblem *>("_subproblem")->mesh().dimension() == 3 ? coupledGradient("disp_z") : _grad_zero)
{
}

Linear::~Linear()
{
}

void
Linear::computeStrain( const unsigned qp,
                       const SymmTensor & total_strain_old,
                       SymmTensor & total_strain_new,
                       SymmTensor & strain_increment )
{
  strain_increment.xx( _grad_disp_x[qp](0) );
  strain_increment.yy( _grad_disp_y[qp](1) );
  strain_increment.zz( _grad_disp_z[qp](2) );
  strain_increment.xy( 0.5*(_grad_disp_x[qp](1)+_grad_disp_y[qp](0)) );
  strain_increment.yz( 0.5*(_grad_disp_y[qp](2)+_grad_disp_z[qp](1)) );
  strain_increment.zx( 0.5*(_grad_disp_z[qp](0)+_grad_disp_x[qp](2)) );
  if (_large_strain)
  {
    strain_increment.xx() += 0.5*(_grad_disp_x[qp](0)*_grad_disp_x[qp](0) +
                                  _grad_disp_y[qp](0)*_grad_disp_y[qp](0) +
                                  _grad_disp_z[qp](0)*_grad_disp_z[qp](0));
    strain_increment.yy() += 0.5*(_grad_disp_x[qp](1)*_grad_disp_x[qp](1) +
                                  _grad_disp_y[qp](1)*_grad_disp_y[qp](1) +
                                  _grad_disp_z[qp](1)*_grad_disp_z[qp](1));
    strain_increment.zz() += 0.5*(_grad_disp_x[qp](2)*_grad_disp_x[qp](2) +
                                  _grad_disp_y[qp](2)*_grad_disp_y[qp](2) +
                                  _grad_disp_z[qp](2)*_grad_disp_z[qp](2));
    strain_increment.xy() += 0.5*(_grad_disp_x[qp](0)*_grad_disp_x[qp](1) +
                                  _grad_disp_y[qp](0)*_grad_disp_y[qp](1) +
                                  _grad_disp_z[qp](0)*_grad_disp_z[qp](1));
    strain_increment.yz() += 0.5*(_grad_disp_x[qp](1)*_grad_disp_x[qp](2) +
                                  _grad_disp_y[qp](1)*_grad_disp_y[qp](2) +
                                  _grad_disp_z[qp](1)*_grad_disp_z[qp](2));
    strain_increment.zx() += 0.5*(_grad_disp_x[qp](2)*_grad_disp_x[qp](0) +
                                  _grad_disp_y[qp](2)*_grad_disp_y[qp](0) +
                                  _grad_disp_z[qp](2)*_grad_disp_z[qp](0));
  }

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}
}
