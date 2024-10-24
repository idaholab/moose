//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorSmoother.h"
#include "MooseUtils.h"
#include "RelationshipManager.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", FunctorSmoother);

template <typename T>
InputParameters
FunctorSmootherTempl<T>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Creates smoother functor(s) using various averaging techniques");
  params.addParam<std::vector<MooseFunctorName>>("functors_in",
                                                 "The name(s) of the functors to smooth");
  params.addParam<std::vector<MooseFunctorName>>("functors_out",
                                                 "The name(s) of the smooth output functors");
  MooseEnum smoothing_techniques("face_average layered_elem_average remove_checkerboard");
  params.addParam<MooseEnum>(
      "smoothing_technique", smoothing_techniques, "How to smooth the functor");

  return params;
}

template <typename T>
FunctorSmootherTempl<T>::FunctorSmootherTempl(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _functors_in(getParam<std::vector<MooseFunctorName>>("functors_in")),
    _functors_out(getParam<std::vector<MooseFunctorName>>("functors_out")),
    _smoothing_technique(getParam<MooseEnum>("smoothing_technique"))
{
  if (_tid == 0)
  {
    // We need one layer of ghosting at least to get neighbor values
    auto & factory = _app.getFactory();
    auto rm_params = factory.getValidParams("ElementSideNeighborLayers");
    rm_params.template set<std::string>("for_whom") = name();
    rm_params.template set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
    rm_params.template set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
        Moose::RelationshipManagerType::COUPLING;
    rm_params.template set<unsigned short>("layers") = 1;
    rm_params.template set<bool>("use_point_neighbors") = false;
    rm_params.template set<bool>("use_displaced_mesh") =
        parameters.template get<bool>("use_displaced_mesh");
    mooseAssert(rm_params.areAllRequiredParamsValid(),
                "All relationship manager parameters should be valid.");
    auto rm_obj = factory.template create<RelationshipManager>(
        "ElementSideNeighborLayers", name() + "_functor_smoothing", rm_params);

    // Delete the resources created on behalf of the RM if it ends up not being added to the
    // App.
    if (!_app.addRelationshipManager(rm_obj))
      factory.releaseSharedObjects(*rm_obj);
  }

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  for (const auto i : index_range(_functors_in))
  {
    // This potentially upcasts functors from non-AD to AD
    const auto & functor_in = getFunctor<typename Moose::ADType<T>::type>(_functors_in[i]);

    // We always add the AD type
    addFunctorProperty<typename Moose::ADType<T>::type>(
        _functors_out[i],
        [this, &functor_in](const auto & r, const auto & t) -> typename Moose::ADType<T>::type
        {
          typename Moose::ADType<T>::type base_value = functor_in(r, t);
          typename Moose::ADType<T>::type average = 0;

          const Elem * r_elem = nullptr;
          if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
            r_elem = r.elem;

          // Handle the single isolated element case
          if (r_elem && r_elem->n_neighbors() == 0)
            return base_value;

          // If these techniques are of interest elsewhere, they should be moved to a
          // FunctorAveragingUtils file and namespaced
          if (_smoothing_technique == FACE_AVERAGE)
          {
            if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
            {
              unsigned int n_faces = 0;
              for (const auto side_index : r_elem->side_index_range())
              {
                auto fi = _mesh.faceInfo(r_elem, side_index);
                if (!fi)
                  fi =
                      _mesh.faceInfo(r_elem->neighbor_ptr(side_index),
                                     r_elem->neighbor_ptr(side_index)->which_neighbor_am_i(r_elem));
                Moose::FaceArg face_arg{
                    fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};
                if (face_arg.fi)
                {
                  average += functor_in(face_arg, t);
                  n_faces++;
                }
              }
              average /= n_faces;
            }
            else
              // a conceptually simple option here would be to use the smoothed version of the
              // ElemArg to compute all the other functor arguments. Maybe with the same smoothing
              // on the gradient calls
              mooseError("Face averaging smoothing has only been defined for the ElemArg functor "
                         "argument, not for ",
                         MooseUtils::prettyCppType(&r),
                         ". Please contact a MOOSE developer or implement it yourself.");
          }
          else if (_smoothing_technique == LAYERED_AVERAGE)
          {
            if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
            {
              unsigned int n_neighbors = 0;
              for (const auto neighbor : r_elem->neighbor_ptr_range())
                if (neighbor)
                {
                  n_neighbors++;
                  average += functor_in(Moose::ElemArg{neighbor, false}, t);
                }
              average /= n_neighbors;
            }
            else
              mooseError("Element layered averaging smoothing has only been defined for the "
                         "ElemArg functor argument, not for ",
                         MooseUtils::prettyCppType(&r),
                         ". Please contact a MOOSE developer or implement it yourself.");
          }
          else
          {
            if constexpr (std::is_same_v<const Moose::ElemArg &, decltype(r)>)
            {
              // We average the local value with the average of the two values furthest away
              // among the neighbors. For a checkerboarding but overall linear evolution, this will
              // compute the average of the "high" and "low" profiles
              average += base_value / 2;

              // Find the neighbor with the furthest value
              typename Moose::ADType<T>::type extreme_value = 0;
              typename Moose::ADType<T>::type delta = 0;

              unsigned int furthest_one = 0;
              unsigned int num_neighbors = 0;
              for (const auto side_index : r_elem->side_index_range())
              {
                auto neighbor = r_elem->neighbor_ptr(side_index);
                if (neighbor)
                {
                  num_neighbors++;
                  auto neighbor_value = functor_in(Moose::ElemArg{neighbor, false}, t);
                  if (std::abs(neighbor_value - base_value) > delta)
                  {
                    furthest_one = side_index;
                    extreme_value = neighbor_value;
                    delta = std::abs(neighbor_value - base_value);
                  }
                }
              }

              // We're on a boundary in 1D, or maybe an odd shape corner in 2D
              if (num_neighbors == 1)
              {
                average += extreme_value / 2;
                return average;
              }
              else
                average += extreme_value / 4;

              // Get the value from the neighbor opposite the furthest-value neighbor
              try
              {
                auto opposite_side = r_elem->opposite_side(furthest_one);
                auto neighbor = r_elem->neighbor_ptr(opposite_side);
                if (neighbor)
                {
                  average += functor_in(Moose::ElemArg{neighbor, false}, t) / 4;
                }
                else
                  // We're probably at a boundary
                  average += extreme_value / 4;
              }
              // if there is no opposite side (for example a triangle)
              catch (libMesh::LogicError & e)
              {
                // find the second furthest
                delta = 0;
                typename Moose::ADType<T>::type second_extreme_value = 0;

                for (const auto side_index : r_elem->side_index_range())
                {
                  auto neighbor = r_elem->neighbor_ptr(side_index);
                  auto neighbor_value =
                      neighbor ? functor_in(Moose::ElemArg{neighbor, false}, t) : base_value;
                  if (std::abs(neighbor_value - base_value) > delta && side_index != furthest_one)
                  {
                    second_extreme_value = neighbor_value;
                    delta = std::abs(neighbor_value - base_value);
                  }
                }
                average += second_extreme_value / 4;
              }
            }
            else
              mooseError("Checkerboard removal smoothing has only been defined for the "
                         "ElemArg functor argument, not for ",
                         MooseUtils::prettyCppType(&r),
                         ". Please contact a MOOSE developer or implement it yourself.");
          }
          return average;
        },
        clearance_schedule);
  }
}

template class FunctorSmootherTempl<Real>;
