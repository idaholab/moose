/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeAxisymmetricRZSmallStrain.h"

template<>
InputParameters validParams<ComputeAxisymmetricRZSmallStrain>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  params.addClassDescription("Compute a small strain in an Axisymmetric geometry. Note '_disp_x' refers to radial displacement and '_disp_y' refers to axial displacement and the coord_type = RZ.");
  return params;
}

ComputeAxisymmetricRZSmallStrain::ComputeAxisymmetricRZSmallStrain(const std::string & name,
                                                 InputParameters parameters) :
    ComputeStrainBase(name, parameters),
    _disp_x(coupledValue("disp_x")),
    _stress_free_strain(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "stress_free_strain")),
    _T_old(coupledValueOld("temperature"))
{
}


void
ComputeAxisymmetricRZSmallStrain::computeProperties()
{

  if (_assembly.coordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to COORD_RZ for Axisymmetric geometries.");

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //Step through calculating the deformation gradient
    //  Note: x_disp is the radial displacement, y_disp is the axial displacement
    _total_strain[_qp](0,0) = _grad_disp_x[_qp](0);
    _total_strain[_qp](1,1) = _grad_disp_y[_qp](1);
    _total_strain[_qp](0,1) = ( _grad_disp_x[_qp](1) + _grad_disp_y[_qp](0) ) / 2.0;
    _total_strain[_qp](1,0) = _total_strain[_qp](0,1);  //force the symmetrical strain tensor

    if ( _q_point[_qp](0) != 0.0 )
      _total_strain[_qp](2,2) = _disp_x[_qp] / _q_point[_qp](0);

    else
      _total_strain[_qp](2,2) = 0.0;

    //Remove thermal expansion
    _total_strain[_qp].addIa(-_thermal_expansion_coeff*( _T[_qp] - _T_old[_qp]));

    //Remove the Eigen strain increment
    _total_strain[_qp] -= _stress_free_strain[_qp];

  }
}
