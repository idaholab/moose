//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVRhieChowInterpolator.h"
#include "Reconstructions.h"
#include "NS.h"
#include "Assembly.h"

registerMooseObject("NavierStokesApp", PINSFVRhieChowInterpolator);

InputParameters
PINSFVRhieChowInterpolator::validParams()
{
  auto params = INSFVRhieChowInterpolator::validParams();
  params.addClassDescription("Performs interpolations and reconstructions of body forces and "
                             "porosity and computes the Rhie-Chow face velocities.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  params.addParam<unsigned short>(
      "smoothing_layers",
      0,
      "The number of interpolation-reconstruction operations to perform on the porosity");
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<unsigned short>("layers") =
                                      obj_params.get<unsigned short>("smoothing_layers");
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
                                });
  return params;
}

PINSFVRhieChowInterpolator::PINSFVRhieChowInterpolator(const InputParameters & params)
  : INSFVRhieChowInterpolator(params),
    _eps(const_cast<Moose::Functor<ADReal> &>(getFunctor<ADReal>(NS::porosity))),
    _epss(libMesh::n_threads(), nullptr),
    _smoothing_layers(getParam<unsigned short>("smoothing_layers")),
    _smoothed_eps(_moose_mesh)
{
  if (_smoothing_layers && _eps.wrapsType<MooseVariableBase>())
    paramError(
        NS::porosity,
        "If we are reconstructing porosity, then the input porosity to this user object cannot "
        "be a Moose variable. There are issues with reconstructing Moose variables: 1) initial "
        "conditions are run after user objects initial setup 2) reconstructing from a variable "
        "requires ghosting the solution vectors 3) it's difficult to restrict the face "
        "informations we evaluate interpolations and reconstructions on such that we never query "
        "an algebraically remote element due to things like two-term extrapolated boundary faces "
        "which trigger gradient evaluations which trigger neighbor element evaluation");

  const auto porosity_name = deduceFunctorName(NS::porosity);

  for (const auto tid : make_range(libMesh::n_threads()))
    _epss[tid] = &UserObject::_subproblem.getFunctor<ADReal>(porosity_name, tid, name());
}

void
PINSFVRhieChowInterpolator::meshChanged()
{
  insfvSetup();
  pinsfvSetup();
}

void
PINSFVRhieChowInterpolator::residualSetup()
{
  if (!_initial_setup_done)
  {
    insfvSetup();
    pinsfvSetup();
  }

  _initial_setup_done = true;
}

void
PINSFVRhieChowInterpolator::pinsfvSetup()
{
  if (!_smoothing_layers)
    return;

  const auto & all_fi = _moose_mesh.allFaceInfo();
  _geometric_fi.reserve(all_fi.size());

  for (const auto & fi : all_fi)
    if (isFaceGeometricallyRelevant(fi))
      _geometric_fi.push_back(&fi);

  _geometric_fi.shrink_to_fit();

  const auto saved_do_derivatives = ADReal::do_derivatives;
  ADReal::do_derivatives = true;
  Moose::FV::interpolateReconstruct(
      _smoothed_eps, _eps, _smoothing_layers, false, _geometric_fi, *this);
  ADReal::do_derivatives = saved_do_derivatives;

  _eps.assign(_smoothed_eps);

  const auto porosity_name = deduceFunctorName(NS::porosity);
  for (const auto tid : make_range((unsigned int)(1), libMesh::n_threads()))
  {
    auto & other_epss = const_cast<Moose::Functor<ADReal> &>(
        UserObject::_subproblem.getFunctor<ADReal>(porosity_name, tid, name()));
    other_epss.assign(_smoothed_eps);
  }
}

bool
PINSFVRhieChowInterpolator::isFaceGeometricallyRelevant(const FaceInfo & fi) const
{
  if (&fi.elem() == libMesh::remote_elem)
    return false;

  bool on_us = _sub_ids.count(fi.elem().subdomain_id());

  if (fi.neighborPtr())
  {
    if (&fi.neighbor() == libMesh::remote_elem)
      return false;

    on_us = on_us || _sub_ids.count(fi.neighbor().subdomain_id());
  }

  if (!on_us)
    // Neither the element nor neighbor has a subdomain id on which we are active, so this face is
    // not relevant
    return false;

  //
  // Ok, we've established that either the element or neighbor is active on our subdomains and
  // neither of them are remote elements, so this face is still in the running to be considered
  // relevant. There is one more caveat to be considered. In the case that we are a boundary face,
  // we will generally need a two term expansion to compute our value, which will require a
  // cell-gradient evaluation. If that is the case, then all of our surrounding neighbors cannot be
  // remote. If we are not a boundary face, then at this point we're safe
  //

  if (!Moose::FV::onBoundary(_sub_ids, fi))
    return true;

  const auto & boundary_elem = (fi.neighborPtr() && _sub_ids.count(fi.neighbor().subdomain_id()))
                                   ? fi.neighbor()
                                   : fi.elem();

  for (auto * const neighbor : boundary_elem.neighbor_ptr_range())
  {
    if (!neighbor)
      continue;

    if ((neighbor == libMesh::remote_elem))
      return false;
  }

  // We made it through all the tests!
  return true;
}
