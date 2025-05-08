//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Kernel.h"
#include "MassMatrix.h"
#include "MaterialProperty.h"
#include "Registry.h"
#include "MooseStaticCondensationPreconditioner.h"
#include "NonlinearSystemBase.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", MassMatrix);

void
MassMatrix::setMassMatrixParams(InputParameters & params)
{
  params.set<MultiMooseEnum>("vector_tags") = "";
  params.set<MultiMooseEnum>("matrix_tags") = "";
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");
  params.set<bool>("matrix_only") = true;
}

InputParameters
MassMatrix::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Computes a finite element mass matrix");
  setMassMatrixParams(params);
  params.addParam<MaterialPropertyName>("density", 1, "The material property defining the density");
  return params;
}

MassMatrix::MassMatrix(const InputParameters & parameters)
  : Kernel(parameters), _density(getMaterialProperty<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
  if (_sys.system().has_static_condensation() &&
      dynamic_cast<const MooseStaticCondensationPreconditioner *>(
          cast_ref<NonlinearSystemBase &>(_sys).getPreconditioner()) &&
      _var.getContinuity() == libMesh::DISCONTINUOUS)
    mooseError("Elemental mass matrices likely don't make sense when using static condensation");
}

Real
MassMatrix::computeQpResidual()
{
  mooseError("Residual should not be calculated for the MassMatrix kernel");
}

Real
MassMatrix::computeQpJacobian()
{
  return _test[_i][_qp] * _density[_qp] * _phi[_j][_qp];
}
