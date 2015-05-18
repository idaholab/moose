/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeIncrementalSmallStrain.h"

template<>
InputParameters validParams<ComputeIncrementalSmallStrain>()
{
  InputParameters params = validParams<ComputeSmallStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for small strains.");
  return params;
}

ComputeIncrementalSmallStrain::ComputeIncrementalSmallStrain(const std::string & name,
                                                 InputParameters parameters) :
    ComputeSmallStrain(name, parameters),
    _strain_rate(declareProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _total_strain_old(declarePropertyOld<RankTwoTensor>("total_strain")),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _deformation_gradient(declareProperty<RankTwoTensor>(_base_name + "deformation gradient")),
    _stress_free_strain_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "stress_free_strain_increment")),
    _T_old(coupledValueOld("temperature"))
{
}

void
ComputeIncrementalSmallStrain::initQpStatefulProperties()
{
  ComputeSmallStrain::initQpStatefulProperties();

  _strain_rate[_qp].zero();
  _strain_increment[_qp].zero();
  _rotation_increment[_qp].zero();
  _rotation_increment[_qp].addIa(1.0); // this remains constant
  _deformation_gradient[_qp].zero();
  _total_strain_old[_qp] = _total_strain[_qp];
}


void
ComputeIncrementalSmallStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //Deformation gradient
    RankTwoTensor A(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp]); //Deformation gradient
    RankTwoTensor Fbar(_grad_disp_x_old[_qp], _grad_disp_y_old[_qp], _grad_disp_z_old[_qp]); //Old Deformation gradient

    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0); //Gauss point deformation gradient

    A -= Fbar; // A = grad_disp - grad_disp_old

    _strain_increment[_qp] = 0.5*(A + A.transpose());

    //Remove thermal expansion
    _strain_increment[_qp].addIa(-_thermal_expansion_coeff*( _T[_qp] - _T_old[_qp]));

    //Remove the Eigen strain increment
    _strain_increment[_qp] -= _stress_free_strain_increment[_qp];

    // strain rate
    _strain_rate[_qp] = _strain_increment[_qp]/_t_step;

    //Update strain in intermediate configuration: rotations are not needed
    _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];
  }
}
