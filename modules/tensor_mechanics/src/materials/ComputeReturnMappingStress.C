/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeReturnMappingStress.h"

#include "StressUpdateBase.h"

template<>
InputParameters validParams<ComputeReturnMappingStress>()
{
  InputParameters params = validParams<ComputeFiniteStrainElasticStress>();
  params.addClassDescription("Compute stress using a radial return mapping implementation for creep or creep combined with plasticity");
  params.addParam<unsigned int>("max_iterations", 30, "Maximum number of the stress update iterations over the stress change after all update materials are called");
  params.addParam<Real>("relative_tolerance", 1e-5, "Relative convergence tolerance for the stress update iterations over the stress change after all update materials are called");
  params.addParam<Real>("absolute_tolerance", 1e-5, "Absolute convergence tolerance for the stress update iterations over the stress change after all update materials are called");
  params.addParam<bool>("output_iteration_info", false, "Set to true to output stress update iteration information over the stress change");
  params.addRequiredParam<std::vector<MaterialName> >("return_mapping_models", "The material objects to use to calculate stress. Note: specify creep models first and plasticity models second.");
  return params;
}

ComputeReturnMappingStress::ComputeReturnMappingStress(const InputParameters & parameters) :
    ComputeFiniteStrainElasticStress(parameters),
    _max_its(parameters.get<unsigned int>("max_iterations")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
{
}

void
ComputeReturnMappingStress::initialSetup()
{
  std::vector<MaterialName> models = getParam<std::vector<MaterialName> >("return_mapping_models");
  for (unsigned int i = 0; i < models.size(); ++i)
  {
    StressUpdateBase * rrr = dynamic_cast<StressUpdateBase *>(&getMaterialByName(models[i]));
    if (rrr)
      _models.push_back(rrr);
    else
      mooseError("Model " + models[i] + " is not compatible with ComputeReturnMappingStress");
  }
}

void
ComputeReturnMappingStress::computeQpStress()
{
  // Nothing to update during the first time step
  // ComputeQpStress is not called during the zeroth time step, so no need to guard against _t_step == 0

  RankTwoTensor strain_increment = _strain_increment[_qp];
  RankTwoTensor stress_new;
  updateQpStress(strain_increment, stress_new);
  _elastic_strain[_qp] = _rotation_increment[_qp] * (strain_increment + _elastic_strain_old[_qp]) * _rotation_increment[_qp].transpose();
  _stress[_qp] = _rotation_increment[_qp] * stress_new * _rotation_increment[_qp].transpose();

  //Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; //This is NOT the exact jacobian
}

void
ComputeReturnMappingStress::updateQpStress(RankTwoTensor & strain_increment,
                                          RankTwoTensor & stress_new)
{
  if (_output_iteration_info == true)
  {
    _console
      << std::endl
      << "iteration output for ComputeReturnMappingStress solve:"
      << " time=" <<_t
      << " int_pt=" << _qp
      << std::endl;
  }

  // compute trial stress
  RankTwoTensor stress_last_iteration = _elasticity_tensor[_qp] * (strain_increment + _elastic_strain_old[_qp]);

  RankTwoTensor inelastic_strain_increment, elastic_strain_increment;
  inelastic_strain_increment.zero();

  Real equivalent_delta_stress, first_equivalent_delta_stress;
  unsigned int counter = 0;

  do
  {
    elastic_strain_increment = strain_increment;
    stress_new = _elasticity_tensor[_qp] * (elastic_strain_increment - inelastic_strain_increment + _elastic_strain_old[_qp]);

    for (unsigned i_rmm =0; i_rmm < _models.size(); ++i_rmm)
    {
      _models[i_rmm]->setQp(_qp);
      _models[i_rmm]->updateStress(elastic_strain_increment,
                                    inelastic_strain_increment,
                                    stress_new);
    }

    // now check convergence in the stress:
    // once the change in stress is within tolerance between two iterations through all the Recompute materials
    // consider the stress to be converged
    RankTwoTensor delta_stress(stress_last_iteration - stress_new);
    equivalent_delta_stress = std::sqrt(delta_stress.doubleContraction(delta_stress));
    if (counter == 0)
      first_equivalent_delta_stress = equivalent_delta_stress;

    stress_last_iteration = stress_new;

    if (_output_iteration_info == true)
    {
      _console
        << "stress iteration number = " << counter << "\n"
        << " relative equivalent delta stress = " << (0 == first_equivalent_delta_stress ? 0 : equivalent_delta_stress/first_equivalent_delta_stress) << "\n"
        << " stress convergence relative tolerance = "  << _relative_tolerance <<"\n"
        << " absolute equivalent delta stress = " << equivalent_delta_stress << "\n"
        << " stress converengen absolute tolerance = "  << _absolute_tolerance
        << std::endl;
    }
    ++counter;
  }
  while (counter < _max_its &&
          equivalent_delta_stress > _absolute_tolerance &&
          (equivalent_delta_stress / first_equivalent_delta_stress) > _relative_tolerance &&
          _models.size() != 1);

  if (counter == _max_its &&
      equivalent_delta_stress > _absolute_tolerance &&
      (equivalent_delta_stress / first_equivalent_delta_stress) > _relative_tolerance)
    mooseError("Max stress iteration hit during ComputeReturnMappingStress solve!");

  strain_increment = elastic_strain_increment;
}
