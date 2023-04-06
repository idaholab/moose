//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestFaceToCellReconstruction.h"

// MOOSE includes
#include "MooseMesh.h"
#include "Assembly.h"

registerMooseObject("NavierStokesTestApp", TestFaceToCellReconstruction);

InputParameters
TestFaceToCellReconstruction::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Computes reconstruction error for face centered functor.");
  return params;
}

TestFaceToCellReconstruction::TestFaceToCellReconstruction(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _reconstruction_error(0.0),
    _face_values(_subproblem.mesh(), _subproblem.mesh().meshSubdomains(), "face_values")
{
}

void
TestFaceToCellReconstruction::initialize()
{
  const auto state = determineState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    const auto & face_center = fi->faceCentroid();
    RealVectorValue face_value(
        -sin(face_center(0)) * cos(face_center(1)), cos(face_center(0)) * sin(face_center(1)), 0);
    _face_values[fi->id()] = face_value;
  }

  // Loop over the local elements if we have distributed tests
  for (const Elem * elem : _fe_problem.mesh().getMesh().active_local_element_ptr_range())
  {
    const auto elem_arg = makeElemArg(elem);
    const Point & elem_centroid = _fe_problem.mesh().elemInfo(elem->id()).centroid();
    const Real elem_volume = this->_assembly.elementVolume(elem);

    RealVectorValue exact_value(-sin(elem_centroid(0)) * cos(elem_centroid(1)),
                                cos(elem_centroid(0)) * sin(elem_centroid(1)),
                                0);

    RealVectorValue diff = exact_value - _face_values(elem_arg, state);
    _reconstruction_error += diff * diff * elem_volume;
  }

  // We collect and sum the values across every process
  gatherSum(_reconstruction_error);

  _reconstruction_error = sqrt(_reconstruction_error);
}

PostprocessorValue
TestFaceToCellReconstruction::getValue()
{
  return _reconstruction_error;
}
