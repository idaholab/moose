#include "PlaneStrain.h"

namespace Elk
{
namespace SolidMechanics
{

PlaneStrain::PlaneStrain(const std::string & name,
                         InputParameters parameters)
  :Element(name, parameters),
   _large_strain(getParam<bool>("large_strain")),
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


}
}
