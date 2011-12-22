#include "Linear.h"

#include "SymmIsotropicElasticityTensorRZ.h"
#include "MaterialModel.h"
#include "Problem.h"
#include "VolumetricModel.h"


namespace Elk
{
namespace SolidMechanics
{

Linear::Linear(const std::string & name,
                               InputParameters parameters)
  :Element(name, parameters),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGradient("disp_z") : _grad_zero)
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

  total_strain_new = strain_increment;

  strain_increment -= total_strain_old;
}


}
}
