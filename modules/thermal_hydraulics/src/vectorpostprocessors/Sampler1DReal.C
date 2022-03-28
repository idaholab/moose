//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Sampler1DReal.h"

registerMooseObject("ThermalHydraulicsApp", Sampler1DReal);
registerMooseObject("ThermalHydraulicsApp", ADSampler1DReal);

template <bool is_ad>
InputParameters
Sampler1DRealTempl<is_ad>::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params += BlockRestrictable::validParams();

  params.addRequiredParam<std::vector<std::string>>(
      "property", "Names of the material properties to be output along a line");

  // This parameter exists in BlockRestrictable, but it is made required here
  // because it is undesirable to use the default, which is to add all blocks.
  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) for which this object will be applied");

  params.addClassDescription(
      "Samples material properties at all quadrature points in mesh block(s)");

  return params;
}

template <bool is_ad>
Sampler1DRealTempl<is_ad>::Sampler1DRealTempl(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    BlockRestrictable(this),
    _mesh(_subproblem.mesh()),
    _qrule(_assembly.qRule()),
    _q_point(_assembly.qPoints())
{
  std::vector<std::string> material_property_names = getParam<std::vector<std::string>>("property");
  for (unsigned int i = 0; i < material_property_names.size(); ++i)
  {
    if (!hasGenericMaterialProperty<Real, is_ad>(material_property_names[i]))
      mooseError("The material property '" + material_property_names[i] + "' does not exist.");
    _material_properties.push_back(
        &getGenericMaterialProperty<Real, is_ad>(material_property_names[i]));
  }

  SamplerBase::setupVariables(material_property_names);
}

template <bool is_ad>
void
Sampler1DRealTempl<is_ad>::initialize()
{
  SamplerBase::initialize();
}

template <bool is_ad>
void
Sampler1DRealTempl<is_ad>::execute()
{
  std::vector<Real> values(_material_properties.size());

  std::set<unsigned int> needed_mat_props;
  const std::set<unsigned int> & mp_deps = getMatPropDependencies();
  needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

  ConstElemRange & elem_range = *(_mesh.getActiveLocalElementRange());
  for (typename ConstElemRange::const_iterator el = elem_range.begin(); el != elem_range.end();
       ++el)
  {
    const Elem * elem = *el;

    if (elem->processor_id() != processor_id())
      continue;

    if (!hasBlocks(elem->subdomain_id()))
      continue;

    _subproblem.setCurrentSubdomainID(elem, _tid);
    _subproblem.prepare(elem, _tid);
    _subproblem.reinitElem(elem, _tid);

    // Set up Sentinel class so that, even if reinitMaterials() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
    _fe_problem.reinitMaterials(elem->subdomain_id(), _tid);

    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      for (unsigned int j = 0; j < _material_properties.size(); ++j)
        values[j] = MetaPhysicL::raw_value((*_material_properties[j])[qp]);

      // use the "x" coordinate as the "id"; at this time, it is not used for anything
      addSample(_q_point[qp], _q_point[qp](0), values);
    }
  }
  _fe_problem.clearActiveMaterialProperties(_tid);
}

template <bool is_ad>
void
Sampler1DRealTempl<is_ad>::finalize()
{
  SamplerBase::finalize();
}
