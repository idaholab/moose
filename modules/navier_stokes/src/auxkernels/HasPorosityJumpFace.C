//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HasPorosityJumpFace.h"
#include "MooseMesh.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", HasPorosityJumpFace);

InputParameters
HasPorosityJumpFace::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Shows whether an element has any attached porosity jump faces");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  return params;
}

HasPorosityJumpFace::HasPorosityJumpFace(const InputParameters & parameters)
  : AuxKernel(parameters), _eps(getFunctor<ADReal>(NS::porosity))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
HasPorosityJumpFace::computeValue()
{
  for (const auto s : _current_elem->side_index_range())
    if (const Elem * const neighbor = _current_elem->neighbor_ptr(s))
    {
      const FaceInfo * const fi =
          Moose::FV::elemHasFaceInfo(*_current_elem, neighbor)
              ? _mesh.faceInfo(_current_elem, s)
              : _mesh.faceInfo(neighbor, neighbor->which_neighbor_am_i(_current_elem));
      mooseAssert(fi, "This should be non-null");
      if (std::get<0>(NS::isPorosityJumpFace(_eps, *fi, determineState())))
        return 1;
    }

  return 0;
}
