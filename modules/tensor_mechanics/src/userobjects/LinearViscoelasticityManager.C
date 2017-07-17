/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                        Grizzly                               */
/*                                                              */
/*           (c) 2015 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "LinearViscoelasticityManager.h"
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<LinearViscoelasticityManager>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<std::string>("viscoelastic_model","name of the LinearViscoelasticityBase object to manage");
  params.addParam<std::string>("stress_name","stress","name of the stress tensor used for the viscoelastic update");
  params.addParam<std::string>("strain_name","mechanical_strain","name of the strain tensor used for the viscoelastic update");
  params.set<MultiMooseEnum>("execute_on") = "timestep_begin timestep_end";
  params.suppressParameter<MultiMooseEnum>("execute_on");
  return params;
}

LinearViscoelasticityManager::LinearViscoelasticityManager(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _stress_name(getParam<std::string>("stress_name")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_stress_name)),
    _strain_name(getParam<std::string>("strain_name")),
    _strain(getMaterialPropertyByName<RankTwoTensor>(_stress_name)),
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

  if (_mi_feproblem.getCurrentExecuteOnFlag() == EXEC_TIMESTEP_END)
  {
    for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
      _viscoelastic_model->updateQpApparentProperties(_qp, _strain[_qp], _stress[_qp]);
  }
}


void
LinearViscoelasticityManager::initialize()
{
  std::shared_ptr<Material> test = _mi_feproblem.getMaterial( _viscoelastic_model_name, _material_data_type, _mi_tid, true );

  if (!test)
    mooseError(_viscoelastic_model_name + " does not exist");

  _viscoelastic_model = std::dynamic_pointer_cast<LinearViscoelasticityBase>(test);

  if (!_viscoelastic_model)
      mooseError(_viscoelastic_model_name + " is not a LinearViscoelasticityBase object");
}

