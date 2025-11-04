//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatFluxBC.h"

registerMooseObject("HeatTransferApp", ADConvectiveHeatFluxBC);

InputParameters
ADConvectiveHeatFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficient given by material properties.");
  // Using material properties
  params.addParam<MaterialPropertyName>("T_infinity",
                                        "Material property for far-field temperature");
  params.addParam<MaterialPropertyName>("heat_transfer_coefficient",
                                        "Material property for heat transfer coefficient");
  // Using functors
  params.addParam<MooseFunctorName>("T_infinity_functor", "Functor for far-field temperature");
  params.addParam<MooseFunctorName>("heat_transfer_coefficient_functor",
                                    "Functor for heat transfer coefficient");
  // In the case where we are coupling a FV variable (via functors) that are block restricted on the
  // neighboring side of boundary, we need to have two layers of ghosting to do the face
  // interpolation.
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<unsigned short>("layers") =
                                      obj_params.isParamValid("T_infinity_functor") ? 2 : 1;
                                });
  return params;
}

ADConvectiveHeatFluxBC::ADConvectiveHeatFluxBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_infinity(isParamValid("T_infinity") ? &getADMaterialProperty<Real>("T_infinity") : nullptr),
    _htc(isParamValid("heat_transfer_coefficient")
             ? &getADMaterialProperty<Real>("heat_transfer_coefficient")
             : nullptr),
    _T_infinity_functor(
        isParamValid("T_infinity_functor") ? &getFunctor<ADReal>("T_infinity_functor") : nullptr),
    _htc_functor(isParamValid("heat_transfer_coefficient_functor")
                     ? &getFunctor<ADReal>("heat_transfer_coefficient_functor")
                     : nullptr)
{
  if (_T_infinity || _htc)
  {
    if (_T_infinity_functor)
      paramError("T_infinity_functor",
                 "Either material properties or functors should be specified for both T_infinity "
                 "and the heat transfer coefficient.");
    if (_htc_functor)
      paramError("heat_transfer_coefficient_functor",
                 "Either material properties or functors should be specified for both T_infinity "
                 "and the heat transfer coefficient");
    if (!_htc)
      paramError("heat_transfer_coefficient",
                 "Heat transfer coefficient material property must be specified");
    if (!_T_infinity)
      paramError("T_infinity", "Far field temperature material property must be specified");
  }
  else if (_T_infinity_functor || _htc_functor)
  {
    if (!_htc_functor)
      paramError("heat_transfer_coefficient_functor",
                 "Heat transfer coefficient functor must be specified");
    if (!_T_infinity_functor)
      paramError("T_infinity_functor", "Far field temperature functor must be specified");
  }
  else
    paramError("T_infinity",
               "Far field temperature and heat transfer coefficients must be specified");
}

void
ADConvectiveHeatFluxBC::initialSetup()
{
  if (_T_infinity)
    return;

  bool T_inf_can_use_neighbor = true;
  bool htc_can_use_neighbor = true;
  // Loop over elements on the primary side of all boundaries
  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    const auto & [elem, side, bid] = *bnd_elem;
    // Skip if this boundary is not part of the restriction
    if (!hasBoundary(bid) || (elem->processor_id() != this->processor_id()))
      continue;

    // Use neighbors if the functor is not defined on all primary blocks
    _T_infinity_use_neighbor =
        _T_infinity_use_neighbor || !_T_infinity_functor->hasBlocks(elem->subdomain_id());
    _htc_use_neighbor = _htc_use_neighbor || !_htc_functor->hasBlocks(elem->subdomain_id());

    // Determine if neighbor can be used, just in case
    const auto neighbor = elem->neighbor_ptr(side);
    if (neighbor)
      mooseAssert(
          !neighbor->is_remote(),
          "The neighbor best not be remote because we request at least one layer of ghosting");
    T_inf_can_use_neighbor = T_inf_can_use_neighbor && neighbor &&
                             _T_infinity_functor->hasBlocks(neighbor->subdomain_id());
    htc_can_use_neighbor =
        htc_can_use_neighbor && neighbor && _htc_functor->hasBlocks(neighbor->subdomain_id());
  }

  _communicator.max(_T_infinity_use_neighbor);
  _communicator.max(_htc_use_neighbor);

  const std::string error_msg =
      "Functor must either be defined on all of the primary side of the boundary or on all "
      "of the secondary side.";
  if (_T_infinity_use_neighbor && !T_inf_can_use_neighbor)
    paramError("T_infinity_functor", error_msg);
  if (_htc_use_neighbor && !htc_can_use_neighbor)
    paramError("heat_transfer_coefficient_functor", error_msg);
}

ADReal
ADConvectiveHeatFluxBC::computeQpResidual()
{
  if (_T_infinity)
    return -_test[_i][_qp] * (*_htc)[_qp] * ((*_T_infinity)[_qp] - _u[_qp]);
  else
  {
    // Populate neighbor information, if necessary (optimization on first qp)
    if ((_T_infinity_use_neighbor || _htc_use_neighbor) && _qp == 0)
    {
      _current_neighbor_elem = _current_elem->neighbor_ptr(_current_side);
      mooseAssert(_current_neighbor_elem, "Neighbor element should exist at this point.");
      _current_neighbor_side = _current_neighbor_elem->which_neighbor_am_i(_current_elem);
    }

    // Convenience lambda for getting space argument on either element or neighbor
    const auto get_space_arg = [this](bool use_neighbor) -> Moose::ElemSideQpArg
    {
      if (!use_neighbor)
        return {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
      else
        return {_current_neighbor_elem, _current_neighbor_side, _qp, _qrule, _q_point[_qp]};
    };

    const auto Tinf_space_arg = get_space_arg(_T_infinity_use_neighbor);
    const auto htc_space_arg = get_space_arg(_htc_use_neighbor);
    const auto time_arg = Moose::currentState();
    return -_test[_i][_qp] * (*_htc_functor)(htc_space_arg, time_arg) *
           ((*_T_infinity_functor)(Tinf_space_arg, time_arg) - _u[_qp]);
  }
}
