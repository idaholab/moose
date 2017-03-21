/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CombinedCreepPlasticity.h"

#include "ReturnMappingModel.h"
#include "SymmIsotropicElasticityTensor.h"

template <>
InputParameters
validParams<CombinedCreepPlasticity>()
{
  InputParameters params = validParams<ConstitutiveModel>();

  params.addRequiredParam<std::vector<std::string>>("submodels",
                                                    "List of submodel ConstitutiveModels");

  params.addParam<unsigned int>("max_its", 30, "Maximum number of submodel iterations");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output submodel iteration information");
  params.addParam<Real>(
      "relative_tolerance", 1e-5, "Relative convergence tolerance for combined submodel iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-5, "Absolute convergence tolerance for combined submodel iteration");

  return params;
}

CombinedCreepPlasticity::CombinedCreepPlasticity(const InputParameters & parameters)
  : ConstitutiveModel(parameters),
    _submodels(),
    _max_its(parameters.get<unsigned int>("max_its")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance"))
{
}

void
CombinedCreepPlasticity::initialSetup()
{
  std::vector<SubdomainID> block_id =
      std::vector<SubdomainID>(blockIDs().begin(), blockIDs().end());
  const std::vector<std::string> & submodels = getParam<std::vector<std::string>>("submodels");
  for (unsigned i(0); i < block_id.size(); ++i)
  {
    std::string suffix;
    std::vector<MooseSharedPointer<Material>> const * mats_p;
    if (_bnd)
    {
      mats_p = &_fe_problem.getMaterialWarehouse()[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(
          block_id[i], _tid);
      suffix = "_face";
    }
    else
      mats_p = &_fe_problem.getMaterialWarehouse().getActiveBlockObjects(block_id[i], _tid);

    const std::vector<MooseSharedPointer<Material>> & mats = *mats_p;
    for (unsigned int i_name(0); i_name < submodels.size(); ++i_name)
    {
      bool found = false;
      for (unsigned int j = 0; j < mats.size(); ++j)
      {
        MooseSharedPointer<ReturnMappingModel> rmm =
            MooseSharedNamespace::dynamic_pointer_cast<ReturnMappingModel>(mats[j]);
        if (rmm && rmm->name() == submodels[i_name] + suffix)
        {
          _submodels[block_id[i]].push_back(rmm);
          found = true;
          break;
        }
      }
      if (!found)
      {
        mooseError("Unable to find submodel " + submodels[i_name]);
      }
    }
  }

  ConstitutiveModel::initialSetup();
}

void
CombinedCreepPlasticity::computeStress(const Elem & current_elem,
                                       unsigned qp,
                                       const SymmElasticityTensor & elasticityTensor,
                                       const SymmTensor & stress_old,
                                       SymmTensor & strain_increment,
                                       SymmTensor & stress_new)
{
  // Given the stretching, compute the stress increment and add it to the old stress. Also update
  // the creep strain
  // stress = stressOld + stressIncrement
  // creep_strain = creep_strainOld + creep_strainIncrement

  if (_t_step == 0 && !_app.isRestarting())
    return;

  if (_output_iteration_info == true)
  {
    _console << std::endl
             << "iteration output for CombinedCreepPlasticity solve:"
             << " time=" << _t << " temperature=" << _temperature[qp] << " int_pt=" << qp
             << std::endl;
  }

  // compute trial stress
  stress_new = elasticityTensor * strain_increment;
  stress_new += stress_old;

  const SubdomainID current_block = current_elem.subdomain_id();
  const std::vector<MooseSharedPointer<ReturnMappingModel>> & rmm(_submodels[current_block]);
  const unsigned num_submodels = rmm.size();

  SymmTensor inelastic_strain_increment;

  SymmTensor elastic_strain_increment;
  SymmTensor stress_new_last(stress_new);
  Real delS(_absolute_tolerance + 1);
  Real first_delS(delS);
  unsigned int counter(0);

  while (counter < _max_its && delS > _absolute_tolerance &&
         (delS / first_delS) > _relative_tolerance && (num_submodels != 1 || counter < 1))
  {
    elastic_strain_increment = strain_increment;
    stress_new = elasticityTensor * (elastic_strain_increment - inelastic_strain_increment);
    stress_new += stress_old;

    for (unsigned i_rmm(0); i_rmm < num_submodels; ++i_rmm)
    {
      rmm[i_rmm]->computeStress(current_elem,
                                qp,
                                elasticityTensor,
                                stress_old,
                                elastic_strain_increment,
                                stress_new,
                                inelastic_strain_increment);
    }

    // now check convergence
    SymmTensor deltaS(stress_new_last - stress_new);
    delS = std::sqrt(deltaS.doubleContraction(deltaS));
    if (counter == 0)
    {
      first_delS = delS;
    }
    stress_new_last = stress_new;

    if (_output_iteration_info == true)
    {
      _console << "stress_it=" << counter
               << " rel_delS=" << (0 == first_delS ? 0 : delS / first_delS)
               << " rel_tol=" << _relative_tolerance << " abs_delS=" << delS
               << " abs_tol=" << _absolute_tolerance << std::endl;
    }

    ++counter;
  }

  if (counter == _max_its && delS > _absolute_tolerance &&
      (delS / first_delS) > _relative_tolerance)
  {
    mooseError("Max stress iteration hit during CombinedCreepPlasticity solve!");
  }

  strain_increment = elastic_strain_increment;
}

bool
CombinedCreepPlasticity::modifyStrainIncrement(const Elem & current_elem,
                                               unsigned qp,
                                               SymmTensor & strain_increment,
                                               SymmTensor & d_strain_dT)
{
  bool modified = false;
  const SubdomainID current_block = current_elem.subdomain_id();
  const std::vector<MooseSharedPointer<ReturnMappingModel>> & rmm(_submodels[current_block]);
  const unsigned num_submodels = rmm.size();

  for (unsigned i_rmm(0); i_rmm < num_submodels; ++i_rmm)
  {
    modified |= rmm[i_rmm]->modifyStrainIncrement(current_elem, qp, strain_increment, d_strain_dT);
  }
  return modified;
}
