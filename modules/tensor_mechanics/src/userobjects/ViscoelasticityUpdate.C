/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ViscoelasticityUpdate.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ViscoelasticityUpdate>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<std::vector<std::string>>(
      "creep_strain_names", "name of the viscoelastic strain objects to update");
  params.addParam<std::string>(
      "strain_name", "mechanical_strain", "strain used for viscoelastic update");
  params.addParam<std::string>("stress_name", "stress", "stress used for viscoelastic update");
  params.set<MultiMooseEnum>("execute_on") = "linear";
  return params;
}

ViscoelasticityUpdate::ViscoelasticityUpdate(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _strain_name(getParam<std::string>("strain_name")),
    _strain(getMaterialPropertyByName<RankTwoTensor>(_strain_name)),
    _stress_name(getParam<std::string>("stress_name")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_stress_name)),
    _creep_strain_names(getParam<std::vector<std::string>>("creep_strain_names")),
    _creep_strains(_creep_strain_names.size(), NULL)
{
}

void
ViscoelasticityUpdate::execute()
{
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    for (unsigned int i = 0; i < _creep_strain_names.size(); ++i)
      _creep_strains[i]->updateQpViscousStrain(_qp, _strain[_qp], _stress[_qp]);
  }
}

void
ViscoelasticityUpdate::initialize()
{
  for (unsigned int i = 0; i < _creep_strain_names.size(); ++i)
  {
    MooseSharedPointer<Material> test =
        _mi_feproblem.getMaterial(_creep_strain_names[i], _material_data_type, _mi_tid, true);

    if (!test)
      mooseError(_creep_strain_names[i] + " does not exist");

    _creep_strains[i] = MooseSharedNamespace::dynamic_pointer_cast<ComputeCreepStrainBase>(test);

    if (!_creep_strains[i])
      mooseError(_creep_strain_names[i] + " is not a ComputeCreepStrainBase object");
  }
}
