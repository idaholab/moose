//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorDirichletBC.h"

registerMooseObject("MooseApp", FVFunctorDirichletBC);
registerMooseObject("MooseApp", FVADFunctorDirichletBC);

template <bool is_ad>
InputParameters
FVFunctorDirichletBCTempl<is_ad>::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Uses the value of a functor to set a Dirichlet boundary value.");
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor whose value is imposed on the boundary");
  params.addParam<bool>(
      "functor_only_defined_on_other_side",
      false,
      "Whether to evaluate the functor on the other side of the boundary. Note that depending on "
      "the functor type, this may require the 'ghost_layers' parameter to be set. For a FV "
      "variable with two term expansion, three ghost layers are needed.");
  params.addParam<unsigned int>("ghost_layers", 0, "Number of ghost layers needed");

  // We need to add ghosting if we are going to use values from the other side
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned int>("ghost_layers");
        rm_params.set<bool>("use_point_neighbors") = false;
      });

  return params;
}

template <bool is_ad>
FVFunctorDirichletBCTempl<is_ad>::FVFunctorDirichletBCTempl(const InputParameters & parameters)
  : FVDirichletBCBase(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _use_other_side(getParam<bool>("functor_only_defined_on_other_side"))
{
}

template <bool is_ad>
ADReal
FVFunctorDirichletBCTempl<is_ad>::boundaryValue(const FaceInfo & fi,
                                                const Moose::StateArg & state) const
{
  auto sfa = singleSidedFaceArg(&fi);
  if (!_use_other_side)
    return _functor(sfa, state);
  else if (fi.elemPtr() == sfa.face_side)
    return _functor(
        {&fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighborPtr(), nullptr},
        state);
  else
    return _functor(
        {&fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.elemPtr(), nullptr},
        state);
}

template class FVFunctorDirichletBCTempl<false>;
template class FVFunctorDirichletBCTempl<true>;
