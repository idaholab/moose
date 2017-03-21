/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVNoSlopeReconstruction.h"

template <>
InputParameters
validParams<CNSFVNoSlopeReconstruction>()
{
  InputParameters params = validParams<SlopeReconstructionBase>();
  params.addClassDescription(
      "A user object that does no slope reconstruction in multi-dimensions.");
  return params;
}

CNSFVNoSlopeReconstruction::CNSFVNoSlopeReconstruction(const InputParameters & parameters)
  : SlopeReconstructionBase(parameters)
{
}

void
CNSFVNoSlopeReconstruction::reconstructElementSlope()
{
  const Elem * elem = _current_elem;

  /// current element id
  dof_id_type _elementID = elem->id();

  /// number of conserved variables
  unsigned int nvars = 5;

  /// vector for the reconstructed gradients of the conserved variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));

  _rslope[_elementID] = ugrad;
}
