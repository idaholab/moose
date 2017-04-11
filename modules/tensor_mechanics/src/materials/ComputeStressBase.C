/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeStressBase.h"
#include "ComputeElasticityTensorBase.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeStressBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::vector<FunctionName>>(
      "initial_stress",
      "A list of functions describing the initial stress.  If provided, there "
      "must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, "
      "zz components respectively.  If not provided, all components of the "
      "initial stress will be zero");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>("store_stress_old",
                        false,
                        "Parameter which indicates whether the old "
                        "stress state, required for the HHT time "
                        "integration scheme and Rayleigh damping, needs "
                        "to be stored");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ComputeStressBase::ComputeStressBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain(declareProperty<RankTwoTensor>(_base_name + "elastic_strain")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _extra_stress(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "extra_stress")),
    _Jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _store_stress_old(getParam<bool>("store_stress_old")),
    _elasticity_tensor_isotropic_guarantee(OptionalBool::VALUE_UNDEFINED)
{
  // Declares old stress and older stress if the parameter _store_stress_old is true. This parameter
  // can be set from the input file using any of the child classes of ComputeStressBase.

  if (_store_stress_old)
  {
    declarePropertyOld<RankTwoTensor>(_base_name + "stress");
    declarePropertyOlder<RankTwoTensor>(_base_name + "stress");
  }

  const std::vector<FunctionName> & fcn_names(
      getParam<std::vector<FunctionName>>("initial_stress"));
  const unsigned num = fcn_names.size();

  if (!(num == 0 || num == 3 * 3))
    mooseError(
        "Either zero or ",
        3 * 3,
        " initial stress functions must be provided to TensorMechanicsMaterial.  You supplied ",
        num,
        "\n");

  _initial_stress.resize(num);
  for (unsigned i = 0; i < num; ++i)
    _initial_stress[i] = &getFunctionByName(fcn_names[i]);

  if (getParam<bool>("use_displaced_mesh"))
    mooseError("The stress calculator needs to run on the undisplaced mesh.");
}

void
ComputeStressBase::initQpStatefulProperties()
{
  _stress[_qp].zero();
  if (_initial_stress.size() == 3 * 3)
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        _stress[_qp](i, j) = _initial_stress[i * 3 + j]->value(_t, _q_point[_qp]);
  _elastic_strain[_qp].zero();
}

void
ComputeStressBase::computeQpProperties()
{
  computeQpStress();

  // Add in extra stress
  _stress[_qp] += _extra_stress[_qp];
}

bool
ComputeStressBase::isElasticityTensorGuaranteedIsotropic()
{
  // we need to determine this on demand in initialSetup
  if (_elasticity_tensor_isotropic_guarantee == OptionalBool::VALUE_UNDEFINED)
  {
    if (!_fe_problem.startedInitialSetup())
      mooseError("isElasticityTensorGuaranteedIsotropic() needs to be called in initialSetup()");

    _elasticity_tensor_isotropic_guarantee = OptionalBool::VALUE_TRUE;

    // Reference to MaterialWarehouse for testing and retrieving block ids
    const auto & warehouse = _fe_problem.getMaterialWarehouse();

    // Complete set of ids that this object is active
    const auto & ids = blockRestricted() ? blockIDs() : meshBlockIDs();

    // Loop over each id for this object
    for (const auto & id : ids)
    {
      // If block materials exist, look if any declare the elasticity tensor
      if (warehouse.hasActiveBlockObjects(id))
      {
        const std::vector<std::shared_ptr<Material>> & mats = warehouse.getActiveBlockObjects(id);
        for (const auto & mat : mats)
        {
          const auto & mat_props = mat->getSuppliedItems();
          if (mat_props.count(_elasticity_tensor_name))
          {
            auto elastic_mat = dynamic_cast<ComputeElasticityTensorBase *>(mat.get());
            if (elastic_mat && !elastic_mat->isGuaranteedIsotropic())
            {
              // we found at least one material on the set of block we operate on
              // that does _not_ guarantee an isotropic elasticity tensor
              _elasticity_tensor_isotropic_guarantee = OptionalBool::VALUE_FALSE;
              break;
            }
          }
        }
      }
    }
  }

  return _elasticity_tensor_isotropic_guarantee == OptionalBool::VALUE_TRUE;
}
