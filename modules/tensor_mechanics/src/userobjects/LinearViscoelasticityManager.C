/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearViscoelasticityManager.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<LinearViscoelasticityManager>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<std::string>("viscoelastic_model",
                                       "name of the LinearViscoelasticityBase object to manage");
  params.addParam<std::string>(
      "stress_name", "stress", "name of the stress tensor used for the viscoelastic update");
  params.addParam<std::string>("creep_strain_name",
                               "creep_strain",
                               "name of the creep strain tensor used for the viscoelastic update");
  params.addParam<std::string>(
      "elastic_strain_name",
      "elastic_strain",
      "name of the elastic strain tensor used for the viscoelastic update");
  params.set<MultiMooseEnum>("execute_on") = "timestep_begin timestep_end";
  params.suppressParameter<MultiMooseEnum>("execute_on");
  return params;
}

LinearViscoelasticityManager::LinearViscoelasticityManager(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _stress_name(getParam<std::string>("stress_name")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_stress_name)),
    _creep_strain_name(getParam<std::string>("creep_strain_name")),
    _creep_strain(getMaterialPropertyByName<RankTwoTensor>(_creep_strain_name)),
    _elastic_strain_name(getParam<std::string>("elastic_strain_name")),
    _elastic_strain(getMaterialPropertyByName<RankTwoTensor>(_elastic_strain_name)),
    _viscoelastic_model_name(getParam<std::string>("viscoelastic_model")),
    _viscoelastic_model(nullptr)
{
}

void
LinearViscoelasticityManager::execute()
{
  if (_mi_feproblem.getCurrentExecuteOnFlag() == EXEC_TIMESTEP_BEGIN)
  {
    for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
      _viscoelastic_model->recomputeQpApparentProperties(_qp);
  }
}

void
LinearViscoelasticityManager::initialize()
{
  std::shared_ptr<Material> test =
      _mi_feproblem.getMaterial(_viscoelastic_model_name, _material_data_type, _mi_tid, true);

  if (!test)
    mooseError(_viscoelastic_model_name + " does not exist");

  _viscoelastic_model = std::dynamic_pointer_cast<LinearViscoelasticityBase>(test);

  if (!_viscoelastic_model)
    mooseError(_viscoelastic_model_name + " is not a LinearViscoelasticityBase object");
}
