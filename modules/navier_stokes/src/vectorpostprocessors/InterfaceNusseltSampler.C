//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceNusseltSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("NavierStokesApp", InterfaceNusseltSampler);

InputParameters
InterfaceNusseltSampler::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addClassDescription("Calculate the Nusselt number  along a sideset, internal or external.");
  params.addRequiredParam<MooseFunctorName>(
      "fluid_temperature", "The name of the variable temperature variable that this VectorPostprocessor operates on");
  params.addRequiredParam<MooseFunctorName>("characteristic_length", "The characteristic length.");
//  params.addRequiredParam<PostprocessorName>("bulk_temperature", "The bulk temperature postprocessor whose values are to be used.");
  params.addRequiredParam<MooseFunctorName>("bulk_temperature", "The bulk temperature postprocessor whose values are to be used.");
  //params.addRequiredParam<Real>("bulk_temperature", "The bulk temperature postprocessor whose values are to be used.");


  // The value from this VPP is naturally already on every processor
  // TODO: Make this not the case!  See #11415
 // params.set<bool>("_auto_broadcast") = false;

  return params;
}

InterfaceNusseltSampler::InterfaceNusseltSampler(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    NonADFunctorInterface(this),
    _fluid_temp(&getFunctor<Real>("fluid_temperature")),
    _char_length(getFunctor<Real>("characteristic_length")),
    _bulk_temp(getFunctor<Real>("bulk_temperature"))
{
  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables({(*_fluid_temp).functorName()});
}

void
InterfaceNusseltSampler::initialize()
{
  SamplerBase::initialize();
}

void
InterfaceNusseltSampler::execute()
{
  getFaceInfos();
  const auto state = determineState();

  for (const auto & fi : _face_infos)
  {
    mooseAssert((*_fluid_temp).hasFaceSide(*fi, true) || (*_fluid_temp).hasFaceSide(*fi, false),
                "Variable " + (*_fluid_temp).functorName() +
                    " should be defined on one side of the face!");

    bool use_elem_side = _fluid_temp->hasFaceSide(*fi, true);
    const auto * elem = use_elem_side ? fi->elemPtr() : fi->neighborPtr();
    
    const auto face_arg = Moose::FaceArg(
		    {fi, Moose::FV::LimiterType::CentralDifference, true, false, elem, nullptr});
    
    const Real temp_val = (*_fluid_temp)(face_arg, state);
    Real grad_dot_n = _fluid_temp->gradient(face_arg, state) * fi->normal()
	    *_char_length(face_arg,state)/(temp_val - _bulk_temp(face_arg,state));

    _values = {grad_dot_n};
    SamplerBase::addSample(fi->faceCentroid(), elem->id(), _values);
  }
}

void
InterfaceNusseltSampler::finalize()
{
  SamplerBase::finalize();
}

void
InterfaceNusseltSampler::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const InterfaceNusseltSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
