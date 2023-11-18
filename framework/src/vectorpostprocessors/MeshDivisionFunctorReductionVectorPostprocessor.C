//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDivisionFunctorReductionVectorPostprocessor.h"
#include "MeshDivision.h"
#include "MooseMesh.h"
#include "MooseMeshUtils.h"
#include "FEProblemBase.h"

#include "libmesh/mesh_base.h"

registerMooseObject("MooseApp", MeshDivisionFunctorReductionVectorPostprocessor);

InputParameters
MeshDivisionFunctorReductionVectorPostprocessor::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  MooseEnum reduction("integral average min max");
  params.addRequiredParam<MooseEnum>("reduction", reduction, "Reduction operation to apply");
  params.addRequiredParam<std::vector<MooseFunctorName>>("functors",
                                                         "Functors to apply the reduction on");
  params.addRequiredParam<MeshDivisionName>(
      "mesh_division",
      "Mesh division object forming the partition of the elements to perform the reduction within");
  params.addClassDescription("Perform reductions on functors based on a per-mesh-division basis");
  return params;
}

MeshDivisionFunctorReductionVectorPostprocessor::MeshDivisionFunctorReductionVectorPostprocessor(
    const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    NonADFunctorInterface(this),
    _reduction(getParam<MooseEnum>("reduction")),
    _nfunctors(getParam<std::vector<MooseFunctorName>>("functors").size()),
    _mesh_division(_fe_problem.getMeshDivision(getParam<MeshDivisionName>("mesh_division"), _tid))
{
  // Gather the functors
  const auto functor_names = getParam<std::vector<MooseFunctorName>>("functors");
  for (const auto i : make_range(_nfunctors))
    _functors.push_back(&getFunctor<Real>(functor_names[i]));

  // Set up reduction vectors
  const auto ndiv = _mesh_division.getNumDivisions();
  _volumes.resize(ndiv);
  for (const auto i : index_range(functor_names))
  {
    auto & p = declareVector(functor_names[i]);
    p.resize(ndiv);
    _functor_reductions.push_back(&p);
  }
}

void
MeshDivisionFunctorReductionVectorPostprocessor::initialize()
{
  for (auto & reduction : _functor_reductions)
  {
    if (_reduction == ReductionEnum::INTEGRAL || _reduction == ReductionEnum::AVERAGE)
      std::fill(reduction->begin(), reduction->end(), 0);
    else if (_reduction == ReductionEnum::MIN)
      std::fill(reduction->begin(), reduction->end(), std::numeric_limits<Real>::max());
    else if (_reduction == ReductionEnum::MAX)
      std::fill(reduction->begin(), reduction->end(), std::numeric_limits<Real>::min());
    else
      mooseAssert(true, "Unknown reduction type");
  }
  std::fill(_volumes.begin(), _volumes.end(), 0);
}

void
MeshDivisionFunctorReductionVectorPostprocessor::execute()
{
  const auto state_arg = determineState();
  if (hasBlocks(_current_elem->subdomain_id()))
  {
    auto index = _mesh_division.divisionIndex(*_current_elem);
    if (index == MooseMeshDivision::INVALID_DIVISION_INDEX)
    {
      mooseWarning("Spatial value sampled outside of the mesh_division specified at: " +
                   Moose::stringify(*_current_elem));
      return;
    }
    for (unsigned int i = 0; i < _nfunctors; ++i)
      if (_functors[i]->hasBlocks(_current_elem->subdomain_id()))
        for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
        {
          Moose::ElemQpArg elem_qp = {_current_elem, qp, _qrule, _q_point[qp]};
          const auto functor_value = (*_functors[i])(elem_qp, state_arg);
          if (_reduction == ReductionEnum::INTEGRAL || _reduction == ReductionEnum::AVERAGE)
            (*_functor_reductions[i])[index] += _JxW[qp] * _coord[qp] * functor_value;
          else if (_reduction == ReductionEnum::MIN)
          {
            if ((*_functor_reductions[i])[index] > functor_value)
              (*_functor_reductions[i])[index] = functor_value;
          }
          else if (_reduction == ReductionEnum::MAX)
            if ((*_functor_reductions[i])[index] < functor_value)
              (*_functor_reductions[i])[index] = functor_value;
          if (i == 0 && _reduction == ReductionEnum::AVERAGE)
            _volumes[index] += _JxW[qp] * _coord[qp];
        }
  }
}

void
MeshDivisionFunctorReductionVectorPostprocessor::finalize()
{
  for (auto & reduction : _functor_reductions)
    if (_reduction == ReductionEnum::INTEGRAL || _reduction == ReductionEnum::AVERAGE)
      gatherSum(*reduction);
    else if (_reduction == ReductionEnum::MIN)
      gatherMin(*reduction);
    else if (_reduction == ReductionEnum::MAX)
      gatherMax(*reduction);

  if (_reduction == ReductionEnum::AVERAGE)
  {
    gatherSum(_volumes);
    for (unsigned int i_f = 0; i_f < _nfunctors; ++i_f)
      for (const auto i : index_range(*_functor_reductions[i_f]))
        if (!MooseUtils::absoluteFuzzyEqual(_volumes[i], 0))
          (*_functor_reductions[i_f])[i] /= _volumes[i];
        else
          (*_functor_reductions[i_f])[i] = 0;
  }
}

void
MeshDivisionFunctorReductionVectorPostprocessor::threadJoin(const UserObject & s)
{
  const MeshDivisionFunctorReductionVectorPostprocessor & sibling =
      static_cast<const MeshDivisionFunctorReductionVectorPostprocessor &>(s);

  for (unsigned int i_f = 0; i_f < _nfunctors; ++i_f)
    for (const auto i : index_range(*_functor_reductions[i_f]))
    {
      if (_reduction == ReductionEnum::INTEGRAL || _reduction == ReductionEnum::AVERAGE)
        (*_functor_reductions[i_f])[i] += (*sibling._functor_reductions[i_f])[i];
      else if (_reduction == ReductionEnum::MIN)
      {
        if ((*_functor_reductions[i_f])[i] > (*sibling._functor_reductions[i_f])[i])
          (*_functor_reductions[i_f])[i] = (*sibling._functor_reductions[i_f])[i];
      }
      else if (_reduction == ReductionEnum::MAX)
        if ((*_functor_reductions[i_f])[i] < (*sibling._functor_reductions[i_f])[i])
          (*_functor_reductions[i_f])[i] = (*sibling._functor_reductions[i_f])[i];

      // Average-reduction requires the reduction of the volume
      if (i_f == 0 && _reduction == ReductionEnum::AVERAGE)
        _volumes[i] += sibling._volumes[i];
    }
}

Real
MeshDivisionFunctorReductionVectorPostprocessor::spatialValue(const Point & p) const
{
  if (_nfunctors > 1)
    mooseError("The spatialValue user object interface was not conceived for objects that compute "
               "multiple values for a given spatial point. Please specify only one functor");
  const auto index = _mesh_division.divisionIndex(p);
  if (index == MooseMeshDivision::INVALID_DIVISION_INDEX)
    mooseError("Spatial value sampled outside of the mesh_division specified at", p);
  return (*_functor_reductions[0])[index];
}
