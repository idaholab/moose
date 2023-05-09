//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE Includes
#include "MortarConsumerInterface.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MortarData.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"

#include <algorithm>

InputParameters
MortarConsumerInterface::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();
  // On a displaced mesh this will geometrically and algebraically ghost the entire interface
  params.addRelationshipManager(
      "AugmentSparsityOnInterface",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
        rm_params.set<BoundaryName>("secondary_boundary") =
            obj_params.get<BoundaryName>("secondary_boundary");
        rm_params.set<BoundaryName>("primary_boundary") =
            obj_params.get<BoundaryName>("primary_boundary");
        rm_params.set<SubdomainName>("secondary_subdomain") =
            obj_params.get<SubdomainName>("secondary_subdomain");
        rm_params.set<SubdomainName>("primary_subdomain") =
            obj_params.get<SubdomainName>("primary_subdomain");
        rm_params.set<bool>("ghost_point_neighbors") =
            obj_params.get<bool>("ghost_point_neighbors");
        rm_params.set<bool>("ghost_higher_d_neighbors") =
            obj_params.get<bool>("ghost_higher_d_neighbors");
      });

  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addRequiredParam<SubdomainName>("primary_subdomain", "The name of the primary subdomain.");
  params.addRequiredParam<SubdomainName>("secondary_subdomain",
                                         "The name of the secondary subdomain.");
  params.addParam<bool>(
      "periodic",
      false,
      "Whether this constraint is going to be used to enforce a periodic condition. This has the "
      "effect of changing the normals vector for projection from outward to inward facing");

  params.addParam<bool>(
      "debug_mesh",
      false,
      "Whether this constraint is going to enable mortar segment mesh debug information. An exodus"
      "file will be generated if the user sets this flag to true");

  params.addParam<bool>(
      "correct_edge_dropping",
      false,
      "Whether to enable correct edge dropping treatment for mortar constraints. When disabled "
      "any Lagrange Multiplier degree of freedom on a secondary element without full primary "
      "contributions will be set (strongly) to 0.");

  params.addParam<bool>(
      "interpolate_normals",
      true,
      "Whether to interpolate the nodal normals (e.g. classic idea of evaluating field at "
      "quadrature points). If this is set to false, then non-interpolated nodal normals will be "
      "used, and then the _normals member should be indexed with _i instead of _qp");

  params.addParam<bool>("ghost_point_neighbors",
                        false,
                        "Whether we should ghost point neighbors of secondary face elements, and "
                        "consequently also their mortar interface couples.");
  params.addParam<Real>(
      "minimum_projection_angle",
      40.0,
      "Parameter to control which angle (in degrees) is admissible for the creation of mortar "
      "segments.  If set to a value close to zero, very oblique projections are allowed, which "
      "can result in mortar segments solving physics not meaningfully, and overprojection of "
      "primary nodes onto the mortar segment mesh in extreme cases. This parameter is mostly "
      "intended for mortar mesh debugging purposes in two dimensions.");

  params.addParam<bool>(
      "ghost_higher_d_neighbors",
      false,
      "Whether we should ghost higher-dimensional neighbors. This is necessary when we are doing "
      "second order mortar with finite volume primal variables, because in order for the method to "
      "be second order we must use cell gradients, which couples in the neighbor cells.");

  return params;
}

// Standard constructor
MortarConsumerInterface::MortarConsumerInterface(const MooseObject * moose_object)
  : _mci_fe_problem(*moose_object->getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mci_subproblem(*moose_object->getCheckedPointerParam<SubProblem *>("_subproblem")),
    _mci_tid(moose_object->getParam<THREAD_ID>("_tid")),
    _mci_mesh(_mci_subproblem.mesh()),
    // all geometric assembly information should be correct for nl system number 0
    _mci_assembly(_mci_subproblem.assembly(_mci_tid, 0)),
    _mortar_data(_mci_fe_problem.mortarData()),
    _secondary_id(
        _mci_mesh.getBoundaryID(moose_object->getParam<BoundaryName>("secondary_boundary"))),
    _primary_id(_mci_mesh.getBoundaryID(moose_object->getParam<BoundaryName>("primary_boundary"))),
    _secondary_subdomain_id(
        _mci_mesh.getSubdomainID(moose_object->getParam<SubdomainName>("secondary_subdomain"))),
    _primary_subdomain_id(
        _mci_mesh.getSubdomainID(moose_object->getParam<SubdomainName>("primary_subdomain"))),
    _interpolate_normals(moose_object->getParam<bool>("interpolate_normals")),
    _phys_points_secondary(_mci_assembly.qPointsFace()),
    _phys_points_primary(_mci_assembly.qPointsFaceNeighbor()),
    _qrule_msm(_mci_assembly.qRuleMortar()),
    _qrule_face(_mci_assembly.qRuleFace()),
    _lower_secondary_elem(_mci_assembly.lowerDElem()),
    _lower_primary_elem(_mci_assembly.neighborLowerDElem()),
    _JxW_msm(_mci_assembly.jxWMortar()),
    _msm_elem(_mci_assembly.msmElem())
{
  const bool displaced = moose_object->isParamValid("use_displaced_mesh")
                             ? moose_object->getParam<bool>("use_displaced_mesh")
                             : false;

  // Create the mortar interface if it hasn't already been created
  _mci_fe_problem.createMortarInterface(
      std::make_pair(_primary_id, _secondary_id),
      std::make_pair(_primary_subdomain_id, _secondary_subdomain_id),
      displaced,
      moose_object->getParam<bool>("periodic"),
      moose_object->getParam<bool>("debug_mesh"),
      moose_object->getParam<bool>("correct_edge_dropping"),
      moose_object->getParam<Real>("minimum_projection_angle"));

  _amg = &_mci_fe_problem.getMortarInterface(
      std::make_pair(_primary_id, _secondary_id),
      std::make_pair(_primary_subdomain_id, _secondary_subdomain_id),
      displaced);

  const auto & secondary_set = _mortar_data.getHigherDimSubdomainIDs(_secondary_subdomain_id);
  const auto & primary_set = _mortar_data.getHigherDimSubdomainIDs(_primary_subdomain_id);

  std::set_union(secondary_set.begin(),
                 secondary_set.end(),
                 primary_set.begin(),
                 primary_set.end(),
                 std::inserter(_higher_dim_subdomain_ids, _higher_dim_subdomain_ids.begin()));
  _boundary_ids = {_secondary_id, _primary_id};
}

void
MortarConsumerInterface::setNormals()
{
  if (interpolateNormals())
    _normals = amg().getNormals(*_lower_secondary_elem, _qrule_face->get_points());
  else
    _normals = amg().getNodalNormals(*_lower_secondary_elem);
}

void
MortarConsumerInterface::trimDerivative(const dof_id_type remove_derivative_index,
                                        ADReal & dual_number)
{
  auto md_it = dual_number.derivatives().nude_data().begin();
  auto mi_it = dual_number.derivatives().nude_indices().begin();

  auto d_it = dual_number.derivatives().nude_data().begin();

  for (auto i_it = dual_number.derivatives().nude_indices().begin();
       i_it != dual_number.derivatives().nude_indices().end();
       ++i_it, ++d_it)
    if (*i_it != remove_derivative_index)
    {
      *mi_it = *i_it;
      *md_it = *d_it;
      ++mi_it;
      ++md_it;
    }

  std::size_t n_indices = md_it - dual_number.derivatives().nude_data().begin();
  dual_number.derivatives().nude_indices().resize(n_indices);
  dual_number.derivatives().nude_data().resize(n_indices);
}
