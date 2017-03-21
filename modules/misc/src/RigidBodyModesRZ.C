/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RigidBodyModesRZ.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<RigidBodyModesRZ>()
{
  InputParameters params = validParams<NodalUserObject>();
  params.addRequiredParam<std::vector<unsigned int>>(
      "subspace_name", "FEProblemBase subspace containing RZ rigid body modes");
  params.addRequiredParam<std::vector<unsigned int>>(
      "subspace_indices", "Indices of FEProblemBase subspace vectors containg rigid body modes");
  params.addRequiredCoupledVar("disp_r", "r-displacement");
  params.addRequiredCoupledVar("disp_z", "z-displacement");
  return params;
}

RigidBodyModesRZ::RigidBodyModesRZ(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _subspace_name(parameters.get<std::string>("subspace_name")),
    _subspace_indices(parameters.get<std::vector<unsigned int>>("subspace_indices")),
    _disp_r_i(coupled("disp_r")),
    _disp_z_i(coupled("disp_z"))
{
  if (_subspace_indices.size() != 1)
  {
    std::stringstream err;
    err << "Expected 1 RZ rigid body mode, got " << _subspace_indices.size() << " instead\n";
    mooseError(err.str());
  }
  for (unsigned int i = 0; i < _subspace_indices.size(); ++i)
  {
    if (_subspace_indices[i] >= _fe_problem.subspaceDim(_subspace_name))
    {
      std::stringstream err;
      err << "Invalid " << i << "-th " << _subspace_name << " index " << _subspace_indices[i]
          << "; must be < " << _fe_problem.subspaceDim(_subspace_name) << "\n";
      mooseError(err.str());
    }
  }
}

void
RigidBodyModesRZ::execute()
{
  // Currently this only works for Lagrange displacement variables!
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  const Node & node = *_current_node;
  // z-translation mode
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[0];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    unsigned int rdof = node.dof_number(nl.number(), _disp_r_i, 0);
    mode.set(rdof, 0.0);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, 1.0);
  }
}

void
RigidBodyModesRZ::finalize()
{
  // Close the basis vectors
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  for (unsigned int i = 0; i < _subspace_indices.size(); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    mode.close();
  }
}
