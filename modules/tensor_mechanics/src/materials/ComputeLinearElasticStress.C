/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeLinearElasticStress.h"

template<>
InputParameters validParams<ComputeLinearElasticStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for small strains");
  return params;
}

ComputeLinearElasticStress::ComputeLinearElasticStress(const std::string & name,
                                                 InputParameters parameters) :
    ComputeStressBase(name, parameters),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _is_finite_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
{
  if (_is_finite_strain)
    mooseError("This linear elastic stress calculation only works for small strains");
}

void
ComputeLinearElasticStress::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_total_strain[_qp];

  //Assign value for elastic strain, which is equal to the total strain
  _elastic_strain[_qp] = _total_strain[_qp];

  //Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
