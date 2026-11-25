//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDSGShellResultants.h"
#include "ComputeDSGShellStrain.h"

registerMooseObject("SolidMechanicsApp", ComputeDSGShellResultants);

InputParameters
ComputeDSGShellResultants::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the stress resultants of a DSG shell element from its "
                             "generalized strains and its section stiffness.");
  return params;
}

ComputeDSGShellResultants::ComputeDSGShellResultants(const InputParameters & parameters)
  : Material(parameters),
    _generalized_strain(getMaterialProperty<DenseVector<Real>>("dsg_shell_generalized_strain")),
    _section_stiffness(getMaterialProperty<DenseMatrix<Real>>("dsg_shell_section_stiffness")),
    _resultants(declareProperty<DenseVector<Real>>("dsg_shell_resultants")),
    _nodal_generalized_strain(
        getOptionalMaterialProperty<DenseMatrix<Real>>("dsg_shell_nodal_generalized_strain")),
    _nodal_resultants(declareProperty<DenseMatrix<Real>>("dsg_shell_nodal_resultants"))
{
}

void
ComputeDSGShellResultants::computeQpProperties()
{
  // The resultants are the section stiffness times the generalized strains
  _resultants[_qp].resize(DSGShell::n_strains);
  _section_stiffness[_qp].vector_mult(_resultants[_qp], _generalized_strain[_qp]);

  // The same product at the nodes, when the strain material evaluated the strains there; an
  // element quantity, stored at the first quadrature point only
  DenseMatrix<Real> & nodal_resultants = _nodal_resultants[_qp];
  if (_qp != 0 || !_nodal_generalized_strain || _nodal_generalized_strain[_qp].m() == 0)
  {
    nodal_resultants.resize(0, 0);
    return;
  }
  const DenseMatrix<Real> & nodal_strain = _nodal_generalized_strain[_qp];
  const unsigned int n_nodes = nodal_strain.m();
  nodal_resultants.resize(n_nodes, DSGShell::n_strains);
  DenseVector<Real> strain(DSGShell::n_strains);
  DenseVector<Real> resultants(DSGShell::n_strains);
  for (unsigned int node = 0; node < n_nodes; ++node)
  {
    for (unsigned int component = 0; component < DSGShell::n_strains; ++component)
      strain(component) = nodal_strain(node, component);
    _section_stiffness[_qp].vector_mult(resultants, strain);
    for (unsigned int component = 0; component < DSGShell::n_strains; ++component)
      nodal_resultants(node, component) = resultants(component);
  }
}
