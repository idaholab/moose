//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DSGShellNodalResultantsAux.h"
#include "DSGShellNodalResultants.h"

registerMooseObject("SolidMechanicsApp", DSGShellNodalResultantsAux);

InputParameters
DSGShellNodalResultantsAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Writes one stress resultant of the DSG shell elements, averaged at "
                             "the nodes by DSGShellNodalResultants, to a nodal (Lagrange) "
                             "auxiliary variable.");
  // The same names and order as DSGShellResultantsAux
  MooseEnum resultant("membrane_force_11 membrane_force_22 membrane_force_12 bending_moment_11 "
                      "bending_moment_22 bending_moment_12 shear_force_1 shear_force_2 "
                      "drilling_moment");
  params.addRequiredParam<MooseEnum>(
      "resultant",
      resultant,
      "The resultant to write, per unit length and in the element frame: membrane_force_11, "
      "_22 and _12 are the in-plane forces N11, N22 and N12; bending_moment_11, _22 and _12 the "
      "moments M11, M22 and M12, the integrals of the stress times the distance from the "
      "mid-surface, positive when the face the normal points to is in tension (the convention "
      "of ShellResultantsAux); shear_force_1 and _2 the transverse shear forces Q1 and Q2; "
      "drilling_moment the "
      "moment conjugate to the drilling rotation. Direction 1 is the first local axis set by "
      "reference_first_local_direction of the strain material.");
  params.addRequiredParam<UserObjectName>(
      "nodal_resultants",
      "The DSGShellNodalResultants user object that averages the resultants at the nodes. It "
      "must execute on the same flags as this kernel, and the strain material must have "
      "compute_nodal_strains = true.");
  return params;
}

DSGShellNodalResultantsAux::DSGShellNodalResultantsAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _resultant_index(getParam<MooseEnum>("resultant")),
    _nodal_resultants(getUserObject<DSGShellNodalResultants>("nodal_resultants"))
{
  if (!isNodal())
    paramError("variable", "The variable must be a nodal variable.");
}

Real
DSGShellNodalResultantsAux::computeValue()
{
  return _nodal_resultants.nodalValue(_current_node->id(), _resultant_index);
}
