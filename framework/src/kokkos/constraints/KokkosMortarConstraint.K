//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMortarConstraint.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "MortarUtils.h"

#include "libmesh/fe_map.h"
#include "libmesh/quadrature.h"
#include "libmesh/reference_elem.h"

namespace Moose::Kokkos
{

InputParameters
MortarConstraint::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += MortarConsumerInterface::validParams();

  // The Lagrange multiplier is the variable of the constraint, on the lower-dimensional secondary
  // elements
  params.addRequiredParam<VariableName>("secondary_variable",
                                        "Primal variable on the secondary surface.");
  params.addParam<VariableName>(
      "primary_variable",
      "Primal variable on the primary surface. If this parameter is not provided then the primary "
      "variable is the secondary variable.");

  params.addRelationshipManager("AugmentSparsityOnInterface",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
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
  params.addRelationshipManager("GhostHigherDLowerDPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  params.registerBase("Constraint");

  return params;
}

MortarConstraint::Interface::Interface(const MooseObject * object, FEProblemBase & problem)
  : MortarConsumerInterface(object), MortarExecutorInterface(problem)
{
}

MortarConstraint::MortarConstraint(const InputParameters & parameters)
  : ResidualObject(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    _secondary_var(_sys.getVariable(_tid, getParam<VariableName>("secondary_variable"))),
    _primary_var(isParamValid("primary_variable")
                     ? _sys.getVariable(_tid, getParam<VariableName>("primary_variable"))
                     : _secondary_var),
    _interface(std::make_shared<Interface>(this, _fe_problem)),
    _use_dual(_var.useDual())
{
  if (_configuration.displaced())
    paramError("use_displaced_mesh",
               "Kokkos mortar constraints cannot yet be posed on the current configuration.");

  // Sides are indexed by MortarSide: the secondary and primary primal variables and the Lagrange
  // multiplier, all on lower-dimensional elements
  const MooseVariableFieldBase * const vars[3] = {&_secondary_var, &_primary_var, &_var};
  for (const auto side : make_range(3u))
  {
    _side_var[side].init(*vars[side]);
    _side_family[side] = vars[side]->feType().family;
    _side_order[side] = vars[side]->feType().order;
    addMooseVariableDependency(const_cast<MooseVariableFieldBase *>(vars[side]));
  }

  for (const auto row : make_range(3u))
    for (const auto col : make_range(3u))
      _coupled[row][col] =
          _fe_problem.areCoupled(vars[row]->number(), vars[col]->number(), _sys.number());
}

MortarConstraint::MortarConstraint(const MortarConstraint & object)
  : ResidualObject(object),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    _secondary_var(object._secondary_var),
    _primary_var(object._primary_var),
    _interface(object._interface),
    _use_dual(object._use_dual),
    _computing_residual(object._computing_residual),
    _computing_jacobian(object._computing_jacobian),
    _secondary_qp_begin(object._secondary_qp_begin),
    _JxW(object._JxW)
{
  for (const auto side : make_range(3u))
  {
    _elem[side] = object._elem[side];
    _type[side] = object._type[side];
    _xi[side] = object._xi[side];
    _side_var[side] = object._side_var[side];
    _side_fe[side] = object._side_fe[side];
    _side_family[side] = object._side_family[side];
    _side_order[side] = object._side_order[side];
    for (const auto col : make_range(3u))
      _coupled[side][col] = object._coupled[side][col];
  }
}

void
MortarConstraint::packSegments()
{
  const auto & mesh = _fe_problem.mesh();
  const auto & kokkos_mesh = *mesh.getKokkosMesh();
  const auto & amg = _interface->amg();
  auto & assembly = _fe_problem.assembly(0, _sys.number());
  const auto & qrule = assembly.qRuleMortar();
  const auto & JxW_msm = assembly.jxWMortar();
  const auto & coord_msm = assembly.mortarCoordTransformation();
  const auto secondary_sub_elem_index =
      amg.dim() == 3 ? amg.mortarSegmentMesh().get_elem_integer_index("secondary_sub_elem") : 0;
  const auto primary_sub_elem_index =
      amg.dim() == 3 ? amg.mortarSegmentMesh().get_elem_integer_index("primary_sub_elem") : 0;

  const MooseVariableFieldBase * const vars[3] = {&_secondary_var, &_primary_var, &_var};
  for (const auto side : make_range(3u))
    _side_fe[side] = kokkosAssembly().getFETypeID(vars[side]->feType());

  std::vector<dof_id_type> secondary_qp_begin = {0};
  std::vector<ContiguousElementID> elems[3];
  std::vector<libMesh::ElemType> types[3];
  std::vector<Point> xis[3];
  std::vector<Real> JxW;

  // The reference coordinates on an interior parent of a reference point on its lower-dimensional
  // side element, whose nodes are the nodes of that side in order
  const auto parent_point = [](const Elem & lower, const Point & xi)
  {
    const auto parent = lower.interior_parent();
    const auto side = parent->which_side_am_i(&lower);
    const auto & reference_parent = libMesh::ReferenceElem::get(parent->type());
    const auto reference_side = reference_parent.build_side_ptr(side);
    return libMesh::FEMap::map(lower.dim(), reference_side.get(), xi);
  };

  for (const auto & [secondary_id, msm_elems] : amg.secondariesToMortarSegments())
  {
    // The map also lists secondary elements of other processes, which a distributed mesh need not
    // hold
    const auto secondary = mesh.getMesh().query_elem_ptr(secondary_id);
    if (!secondary || secondary->processor_id() != processor_id() || msm_elems.empty())
      continue;

    for (const auto msm_elem : msm_elems)
    {
      _fe_problem.reinitMortarElem(msm_elem, 0);
      const auto & msinfo = libmesh_map_find(amg.mortarSegmentMeshElemToInfo(), msm_elem);

      std::vector<Point> xi1, xi2;
      if (msm_elem->dim() == 1)
        for (const auto qp : make_range(qrule->n_points()))
        {
          const Real eta = qrule->qp(qp)(0);
          xi1.push_back(0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b);
          xi2.push_back(0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b);
        }
      else if (amg.mortar3DQpMapping() == Mortar3DQuadraturePointMapping::REFERENCE_INTERPOLATION)
        Moose::Mortar::mapQPoints3dFromReference(
            *msm_elem, amg.mortarSegmentReferencePoints(*msm_elem), *qrule, xi1, xi2);
      else
      {
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.secondary_elem, secondary_sub_elem_index, *qrule, xi1);
        Moose::Mortar::projectQPoints3d(
            msm_elem, msinfo.primary_elem, primary_sub_elem_index, *qrule, xi2);
      }

      const Elem * const side_elems[3] = {msinfo.secondary_elem->interior_parent(),
                                          msinfo.primary_elem->interior_parent(),
                                          msinfo.secondary_elem};

      for (const auto qp : make_range(qrule->n_points()))
      {
        const Point side_xi[3] = {parent_point(*msinfo.secondary_elem, xi1[qp]),
                                  parent_point(*msinfo.primary_elem, xi2[qp]),
                                  xi1[qp]};

        for (const auto side : make_range(3u))
        {
          elems[side].push_back(kokkos_mesh.getContiguousElementID(side_elems[side]));
          types[side].push_back(side_elems[side]->type());
          xis[side].push_back(side_xi[side]);
        }

        JxW.push_back(JxW_msm[qp] * coord_msm[qp]);
      }
    }

    secondary_qp_begin.push_back(JxW.size());
  }

  const auto n = JxW.size();
  _secondary_qp_begin.create(secondary_qp_begin.size());
  _JxW.create(n);

  for (const auto i : index_range(secondary_qp_begin))
    _secondary_qp_begin[i] = secondary_qp_begin[i];

  for (const auto qp : make_range(n))
    _JxW[qp] = JxW[qp];

  for (const auto side : make_range(3u))
  {
    _elem[side].create(n);
    _type[side].create(n);
    _xi[side].create(n);

    for (const auto qp : make_range(n))
    {
      _elem[side][qp] = elems[side][qp];
      _type[side][qp] = types[side][qp];
      _xi[side][qp] = xis[side][qp];
    }

    _elem[side].copyToDevice();
    _type[side].copyToDevice();
    _xi[side].copyToDevice();
  }

  _secondary_qp_begin.copyToDevice();
  _JxW.copyToDevice();

  _interface->needs_packing = false;
}

void
MortarConstraint::dispatch(const bool residual, const bool jacobian)
{
  if (_interface->needs_packing)
    packSegments();

  if (_secondary_qp_begin.size() < 2)
    return;

  _computing_residual = residual;
  _computing_jacobian = jacobian;

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<MortarLoop>(this, type());

  TeamLaunch launch;
  launch.league_size = _secondary_qp_begin.size() - 1;
  _residual_dispatcher->parallelForTeams(launch);
}

void
MortarConstraint::computeResidual()
{
  dispatch(true, false);
}

void
MortarConstraint::computeJacobian()
{
  dispatch(false, true);
}

void
MortarConstraint::computeResidualAndJacobian()
{
  dispatch(true, true);
}

} // namespace Moose::Kokkos
