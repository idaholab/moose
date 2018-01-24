/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RigidBodyModes3D.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<RigidBodyModes3D>()
{
  InputParameters params = validParams<NodalUserObject>();
  params.addRequiredParam<std::string>("subspace_name",
                                       "FEProblemBase subspace containing rigid body mode vectors");
  params.addParam<std::vector<unsigned int>>(
      "subspace_indices",
      std::vector<unsigned int>(),
      "Indices of FEProblemBase subspace vectors containing rigid body modes");
  params.addParam<std::vector<std::string>>("modes",
                                            std::vector<std::string>(),
                                            "Names of the RigidBody3D modes computed here. Select "
                                            "from: trans_x, trans_y, trans_z, rot_x, rot_y, rot_z");
  params.addRequiredCoupledVar("disp_x", "x-displacement");
  params.addRequiredCoupledVar("disp_y", "y-displacement");
  params.addRequiredCoupledVar("disp_z", "z-displacement");
  // params.addRequiredParam<AuxVariableName>("trans_x_disp_x", "x-displacement's x-component");
  // params.addRequiredParam<AuxVariableName>("trans_x_disp_y", "x-displacement's y-component");
  // params.addRequiredParam<AuxVariableName>("trans_x_disp_z", "x-displacement's z-component");
  // params.addRequiredParam<AuxVariableName>("trans_y_disp_x", "x-displacement's x-component");
  // params.addRequiredParam<AuxVariableName>("trans_y_disp_y", "y-displacement's y-component");
  // params.addRequiredParam<AuxVariableName>("trans_y_disp_z", "z-displacement's z-component");
  // params.addRequiredParam<AuxVariableName>("trans_z_disp_x", "x-displacement's x-component");
  // params.addRequiredParam<AuxVariableName>("trans_z_disp_y", "y-displacement's y-component");
  // params.addRequiredParam<AuxVariableName>("trans_z_disp_z", "z-displacement's z-component");
  // params.addRequiredParam<AuxVariableName>("rot_x_disp_x", "x-rotation's x-component");
  // params.addRequiredParam<AuxVariableName>("rot_x_disp_y", "x-rotation's y-component");
  // params.addRequiredParam<AuxVariableName>("rot_x_disp_z", "x-rotation's z-component");
  // params.addRequiredParam<AuxVariableName>("rot_y_disp_x", "y-rotation's x-component");
  // params.addRequiredParam<AuxVariableName>("rot_y_disp_y", "y-rotation's y-component");
  // params.addRequiredParam<AuxVariableName>("rot_y_disp_z", "y-rotation's z-component");
  // params.addRequiredParam<AuxVariableName>("rot_z_disp_x", "z-rotation's x-component");
  // params.addRequiredParam<AuxVariableName>("rot_z_disp_y", "z-rotation's y-component");
  // params.addRequiredParam<AuxVariableName>("rot_z_disp_z", "z-rotation's z-component");
  return params;
}

RigidBodyModes3D::RigidBodyModes3D(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _subspace_name(parameters.get<std::string>("subspace_name")),
    _subspace_indices(parameters.get<std::vector<unsigned int>>("subspace_indices")),
    _modes(parameters.get<std::vector<std::string>>("modes").begin(),
           parameters.get<std::vector<std::string>>("modes").end()),
    _disp_x_i(coupled("disp_x")),
    _disp_y_i(coupled("disp_y")),
    _disp_z_i(coupled("disp_z"))
{
  const char * all_modes_array[6] = {"trans_x", "trans_y", "trans_z", "rot_x", "rot_y", "rot_z"};
  std::set<std::string> all_modes(all_modes_array, all_modes_array + 6);
  if (_modes.size() == 0)
    _modes = all_modes;
  if (_modes.size() > 6)
  {
    std::stringstream err;
    err << "Expected between 0 and 6 rigid body modes, got " << _modes.size() << " instead\n";
    mooseError(err.str());
  }
  for (std::set<std::string>::const_iterator it = _modes.begin(); it != _modes.end(); ++it)
  {
    if (all_modes.find(*it) == all_modes.end())
    {
      std::stringstream err;
      err << "Invalid 3D rigid body mode " << *it << "; must be one of: ";
      for (std::set<std::string>::iterator it = all_modes.begin(); it != all_modes.end(); ++it)
      {
        if (it != all_modes.begin())
          err << ", ";
        err << *it;
      }
      err << "\n";
      mooseError(err.str());
    }
  }

  if (!_subspace_indices.size())
  {
    _subspace_indices = std::vector<unsigned int>(_fe_problem.subspaceDim(_subspace_name));
    for (unsigned int i = 0; i < _fe_problem.subspaceDim(_subspace_name); ++i)
      _subspace_indices[i] = i;
  }
  if (_subspace_indices.size() != _modes.size())
  {
    std::stringstream err;
    err << "Number of subspace indices " << _subspace_indices.size()
        << " must match the number or rigid body modes " << _modes.size() << "\n";
    mooseError(err.str());
  }

  for (unsigned int i = 0; i < _subspace_indices.size(); ++i)
  {
    unsigned int subspace_dim = _fe_problem.subspaceDim(_subspace_name);
    if (_subspace_indices[i] >= subspace_dim)
    {
      std::stringstream err;
      err << "Invalid " << i << "-th " << _subspace_name << " index " << _subspace_indices[i]
          << "; must be < " << _fe_problem.subspaceDim(_subspace_name) << "\n";
      mooseError(err.str());
    }
  }
}

void
RigidBodyModes3D::execute()
{
  // Set the appropriate dof of the selectedrigid body vectors
  // Currently this only works for Lagrange displacement variables!
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  const Node & node = *_current_node;
  unsigned int i = 0;
  // x-displacement mode
  if (_modes.count("trans_x"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, 1.0);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, 0.0);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, 0.0);
  }
  // y-displacement mode
  if (_modes.count("trans_y"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, 0.0);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, 1.0);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, 0.0);
  }
  // z-displacement mode
  if (_modes.count("trans_z"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, 0.0);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, 0.0);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, 1.0);
  }
  // x-axis rotation mode
  if (_modes.count("rot_x"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    Real y = node(1), z = node(2);
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, 0.0);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, -z);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, y);
  }
  // y-axis rotation mode
  if (_modes.count("rot_y"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    Real x = node(0), z = node(2);
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, z);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, 0);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, -x);
  }
  // z-axis rotation mode
  if (_modes.count("rot_z"))
  {
    std::stringstream postfix;
    postfix << "_" << _subspace_indices[i++];
    NumericVector<Number> & mode = nl.getVector(_subspace_name + postfix.str());
    Real x = node(0), y = node(1);
    unsigned int xdof = node.dof_number(nl.number(), _disp_x_i, 0);
    mode.set(xdof, -y);
    unsigned int ydof = node.dof_number(nl.number(), _disp_y_i, 0);
    mode.set(ydof, x);
    unsigned int zdof = node.dof_number(nl.number(), _disp_z_i, 0);
    mode.set(zdof, 0);
  }
}

void
RigidBodyModes3D::finalize()
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
