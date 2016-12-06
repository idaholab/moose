/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowVariableBase.h"

template<>
InputParameters validParams<PorousFlowVariableBase>()
{
  InputParameters params = validParams<PorousFlowMaterial>();
  params.addParam<bool>("at_nodes", false, "Material properties will be calculated at nodes rather than the usual quadpoints");
  params.addClassDescription("Base class for thermophysical variable materials. Provides pressure and saturation material properties for all phases as required");
  return params;
}

PorousFlowVariableBase::PorousFlowVariableBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<PorousFlowMaterial>(parameters),

    _num_phases(_dictator.numPhases()),
    _num_components(_dictator.numComponents()),
    _num_pf_vars(_dictator.numVariables()),

    _node_number(getMaterialProperty<unsigned int>("PorousFlow_node_number")),

    _porepressure_nodal(getParam<bool>("at_nodes") ? &declareProperty<std::vector<Real> >("PorousFlow_porepressure_nodal") : nullptr),
    _porepressure_nodal_old(getParam<bool>("at_nodes") ? &declarePropertyOld<std::vector<Real> >("PorousFlow_porepressure_nodal") : nullptr),
    _porepressure_qp(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<Real> >("PorousFlow_porepressure_qp") : nullptr),
    _gradp_qp(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure_qp") : nullptr),
    _dporepressure_nodal_dvar(getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar") : nullptr),
    _dporepressure_qp_dvar(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar") : nullptr),
    _dgradp_qp_dgradv(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_porepressure_qp_dgradvar") : nullptr),
    _dgradp_qp_dv(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_porepressure_qp_dvar") : nullptr),

    _saturation_nodal(getParam<bool>("at_nodes") ? &declareProperty<std::vector<Real> >("PorousFlow_saturation_nodal") : nullptr),
    _saturation_nodal_old(getParam<bool>("at_nodes") ? &declarePropertyOld<std::vector<Real> >("PorousFlow_saturation_nodal") : nullptr),
    _saturation_qp(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<Real> >("PorousFlow_saturation_qp") : nullptr),
    _grads_qp(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<RealGradient> >("PorousFlow_grad_saturation_qp") : nullptr),
    _dsaturation_nodal_dvar(getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_nodal_dvar") : nullptr),
    _dsaturation_qp_dvar(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar") : nullptr),
    _dgrads_qp_dgradv(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_saturation_qp_dgradvar") : nullptr),
    _dgrads_qp_dv(!getParam<bool>("at_nodes") ? &declareProperty<std::vector<std::vector<RealGradient> > >("dPorousFlow_grad_saturation_qp_dv") : nullptr)
{
  _nodal_material = getParam<bool>("at_nodes");
}

void
PorousFlowVariableBase::initQpStatefulProperties()
{
  /// Resize the material properties which constain pressure and saturation
  if (_nodal_material)
  {
    (*_porepressure_nodal)[_qp].resize(_num_phases);
    (*_dporepressure_nodal_dvar)[_qp].resize(_num_phases);

    (*_saturation_nodal)[_qp].resize(_num_phases);
    (*_dsaturation_nodal_dvar)[_qp].resize(_num_phases);
  }
}

void
PorousFlowVariableBase::computeQpProperties()
{
  if (_nodal_material)
  {
    // do we really need this stuff here?  it seems very inefficient to keep resizing everything!
    (*_porepressure_nodal)[_qp].resize(_num_phases);
    (*_dporepressure_nodal_dvar)[_qp].resize(_num_phases);

    (*_saturation_nodal)[_qp].resize(_num_phases);
    (*_dsaturation_nodal_dvar)[_qp].resize(_num_phases);
  }
  else
  {
    (*_porepressure_qp)[_qp].resize(_num_phases);
    (*_gradp_qp)[_qp].resize(_num_phases);
    (*_dporepressure_qp_dvar)[_qp].resize(_num_phases);
    (*_dgradp_qp_dgradv)[_qp].resize(_num_phases);
    (*_dgradp_qp_dv)[_qp].resize(_num_phases);

    (*_saturation_qp)[_qp].resize(_num_phases);
    (*_grads_qp)[_qp].resize(_num_phases);
    (*_dsaturation_qp_dvar)[_qp].resize(_num_phases);
    (*_dgrads_qp_dgradv)[_qp].resize(_num_phases);
    (*_dgrads_qp_dv)[_qp].resize(_num_phases);
  }

  /// Prepare the derivative matrices with zeroes
  for (unsigned phase = 0; phase < _num_phases; ++phase)
  {
    if (_nodal_material)
    {
      (*_dporepressure_nodal_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dsaturation_nodal_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
    }
    else
    {
      (*_dporepressure_qp_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dgradp_qp_dgradv)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dgradp_qp_dv)[_qp][phase].assign(_num_pf_vars, RealGradient());
      (*_dsaturation_qp_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dgrads_qp_dgradv)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dgrads_qp_dv)[_qp][phase].assign(_num_pf_vars, RealGradient());
    }
  }
}

