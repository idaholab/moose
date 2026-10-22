//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeCartesianLMMechanicalContact.h"
#include "KokkosMortarContactKinematicsUserObject.h"

#include "FEProblemBase.h"

registerMooseObject("ContactApp", KokkosComputeCartesianLMMechanicalContact);

static char
registerKokkosComputeCartesianLMMechanicalContact()
{
  using Object = KokkosComputeCartesianLMMechanicalContact;
  using namespace Moose::Kokkos;
  const std::string name = "KokkosComputeCartesianLMMechanicalContact";

  DispatcherRegistry::addDispatcher<Object::NodeLoop, Object>(name);
  DispatcherRegistry::addTeamDispatcher<Object::MortarLoop, Object>(name);
  DispatcherRegistry::addDispatcher<Object::InactiveLoop, Object>(name);
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_cartesian_lm_mechanical_contact =
    registerKokkosComputeCartesianLMMechanicalContact();

InputParameters
KokkosComputeCartesianLMMechanicalContact::validParams()
{
  InputParameters params = KokkosCartesianMortarConstraintBase::validParams();
  params.addClassDescription("Enforces the normal and Coulomb frictional mortar contact conditions "
                             "at the secondary nodes with a Cartesian Lagrange multiplier.");
  params.addRequiredParam<UserObjectName>(
      "kinematics_uo", "The mortar contact kinematics user object, which must compute the slip.");
  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<Real>("c_t", 1, "Parameter for balancing the size of the slip and the traction");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c and c_t by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.addRequiredRangeCheckedParam<Real>(
      "mu", "mu > 0", "The friction coefficient for the Coulomb friction law");
  params.addParam<Real>(
      "epsilon",
      1e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");

  // The nodal normal at a secondary node depends on every secondary face incident to the node
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<bool>("ghost_point_neighbors");

  return params;
}

KokkosComputeCartesianLMMechanicalContact::KokkosComputeCartesianLMMechanicalContact(
    const InputParameters & parameters)
  : KokkosCartesianMortarConstraintBase(parameters),
    _kinematics_uo(_fe_problem.getKokkosUserObject<KokkosMortarContactKinematicsUserObject>(
        getParam<UserObjectName>("kinematics_uo"))),
    _c(getParam<Real>("c")),
    _c_t(getParam<Real>("c_t")),
    _normalize_c(getParam<bool>("normalize_c")),
    _mu(getParam<Real>("mu")),
    _epsilon(getParam<Real>("epsilon"))
{
  if (!_kinematics_uo.parameters().get<bool>("compute_slip"))
    paramError("kinematics_uo",
               "The mortar contact kinematics user object must set compute_slip = true.");

  _interface_data.requireNodalNormals();
}

KokkosComputeCartesianLMMechanicalContact::KokkosComputeCartesianLMMechanicalContact(
    const KokkosComputeCartesianLMMechanicalContact & object)
  : KokkosCartesianMortarConstraintBase(object),
    _kinematics_uo(object._kinematics_uo),
    _c(object._c),
    _c_t(object._c_t),
    _normalize_c(object._normalize_c),
    _mu(object._mu),
    _epsilon(object._epsilon),
    _gap_vector(object._gap_vector),
    _slip(object._slip),
    _normalization(object._normalization),
    _designated(object._designated)
{
}

void
KokkosComputeCartesianLMMechanicalContact::dispatchMortar(const Moose::Kokkos::ThreadID n_secondary)
{
  if (!n_secondary)
    return;

  _gap_vector = _kinematics_uo.weightedGapVector();
  _slip = _kinematics_uo.weightedSlip();
  _normalization = _kinematics_uo.normalization();
  _designated = _kinematics_uo.designated();

  if (!_node_dispatcher)
    _node_dispatcher = Moose::Kokkos::DispatcherRegistry::build<NodeLoop>(this, type());
  _node_dispatcher->parallelFor(Moose::Kokkos::Policy(0, _geometry.nodal_normal.size()));

  // The derivatives with respect to the weighted gap vector and the slip are summed over the
  // mortar segments of the node
  if (_computing_jacobian)
  {
    if (!_residual_dispatcher)
      _residual_dispatcher = Moose::Kokkos::DispatcherRegistry::build<MortarLoop>(this, type());

    Moose::Kokkos::TeamLaunch launch;
    launch.league_size = n_secondary;
    _residual_dispatcher->parallelForTeams(launch);
  }
}
