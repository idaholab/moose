/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RecomputeGeneralReturnStressIncrement.h"

template<>
InputParameters validParams<RecomputeGeneralReturnStressIncrement>()
{
  InputParameters params = validParams<Material>();

  // Newton Iteration control parameters
  params.addParam<unsigned int>("max_iterations", 30, "Maximum number of newton iterations in the radial return material");
  // params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");
  // params.addParam<bool>("output_iteration_info_on_error", false, "Set true to output the discrete material iteration information when a step fails");
  // params.addParam<Real>("relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  // params.addParam<Real>("absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");

  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");

  return params;
}


RecomputeGeneralReturnStressIncrement::RecomputeGeneralReturnStressIncrement(const InputParameters & parameters) :
    Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _return_stress_increment(declareProperty<RankTwoTensor>("return_stress_increment")),
    _inelastic_strain_increment(declareProperty<RankTwoTensor>("inelastic_strain_increment")),
    _max_its(parameters.get<unsigned int>("max_iterations")),
    // _output_iteration_info(getParam<bool>("output_iteration_info")),
    // _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    // _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    // _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _elasticity_tensor(getMaterialPropertyByName<ElasticityTensorR4>(_base_name + "elasticity_tensor")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress"))
{
  if (_compute)
    mooseError("The return stress increment classes are intended to be iterative materials; add ComputeReturnMappingStress to the input file and set compute = false in " << _name );
}

void
RecomputeGeneralReturnStressIncrement::resetQpProperties()
{
  /// Values here to be set to a constant, ideally zero, as in the initQpProperties method of non-discrete materials
  _return_stress_increment[_qp].zero();
  _inelastic_strain_increment[_qp].zero();
}

void
RecomputeGeneralReturnStressIncrement::computeQpProperties()
{
  // Nothing to update during the first time step, return immediately
  if (_t_step == 0)
    return;

  // Given the stretching, compute the stress increment and add iteration to the old stress. Also update the inelastic strain
  // Compute the stress in the intermediate configuration while retaining the stress history
  _return_stress_increment[_qp] = _elasticity_tensor[_qp] * _strain_increment[_qp] + _stress_old[_qp];

  // Call the specific iterative material model to compute the return stress and
  // inelastic strain increments
  computeStressIncrement();
}
