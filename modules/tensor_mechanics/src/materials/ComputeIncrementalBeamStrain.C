/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeIncrementalBeamStrain.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<ComputeIncrementalBeamStrain>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute a infinitesimal/large strain increment for the beam.");
  params.addRequiredCoupledVar(
      "rotations", "The rotations appropriate for the simulation geometry and coordinate system");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<RealGradient>("y_orientation",
                                        "Orientation of the y direction along "
                                        "with Iyy is provided. This should be "
                                        "perpendicular to the axis of the beam.");
  params.addCoupledVar("area", "Variable containing cross-section area");
  params.addCoupledVar("Ay", "Variable containing first moment of area about y axis");
  params.addCoupledVar("Az", "Variable containing first moment of area about z axis");
  params.addCoupledVar("Iy", "Variable containing second moment of area about y axis");
  params.addCoupledVar("Iz", "Variable containing second moment of area about z axis");
  params.addParam<bool>(
      "large_strain", false, "Set to true if large strain have to be calculated.");
  params.set<bool>("volumetric_locking_correction") = false;

  return params;
}

ComputeIncrementalBeamStrain::ComputeIncrementalBeamStrain(const InputParameters & parameters)
  : Material(parameters),
    _nrot(coupledComponents("rotations")),
    _ndisp(coupledComponents("displacements")),
    _rot_num(3),
    _disp_num(3),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _area(coupledValue("area")),
    _Ay(coupledValue("Ay")),
    _Az(coupledValue("Az")),
    _Iy(coupledValue("Iy")),
    _Iz(coupledValue("Iz")),
    _original_local_config(declareProperty<RankTwoTensor>(_base_name + "original_local_config")),
    _original_length(declareProperty<Real>(_base_name + "original_length")),
    _total_rotation(declareProperty<RankTwoTensor>(_base_name + "total_rotation")),
    _disp_strain_increment(declareProperty<RealVectorValue>(_base_name + "disp_strain_increment")),
    _rot_strain_increment(declareProperty<RealVectorValue>(_base_name + "rot_strain_increment")),
    _material_stiffness(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "material_stiffness")),
    _K11(declareProperty<RankTwoTensor>(_base_name + "Jacobian_11")),
    _K21_cross(declareProperty<RankTwoTensor>(_base_name + "Jacobian_12")),
    _K21(declareProperty<RankTwoTensor>(_base_name + "Jacobian_21")),
    _K22(declareProperty<RankTwoTensor>(_base_name + "Jacobian_22")),
    _K22_cross(declareProperty<RankTwoTensor>(_base_name + "Jacobian_22_cross")),
    _large_strain(getParam<bool>("large_strain")),
    _grad_disp_0_local_t(3, 0.0),
    _grad_rot_0_local_t(3, 0.0),
    _avg_rot_local_t(3, 0.0)
{
  // Checking for consistency between mesh dimension and length of the provided displacements vector
  if (_ndisp != _nrot && _ndisp != _mesh.dimension())
    mooseError("The number of variables supplied in 'displacements' and 'rotations' must match the "
               "mesh dimension.");

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    MooseVariable * disp_variable = getVar("displacements", i);
    _disp_num[i] = disp_variable->number();

    MooseVariable * rot_variable = getVar("rotations", i);
    _rot_num[i] = rot_variable->number();
  }

  if (_large_strain && (_Ay[0] > 0.0 || _Ay[1] > 0.0 || _Az[0] > 0.0 || _Az[1] > 0.0))
    mooseError("Error in ComputeIncrementalBeamStrain: Large strain caclulation does not currently "
               "support asymmetric beam configurations with non-zero first or thrid moments of "
               "area.");
}

void
ComputeIncrementalBeamStrain::initQpStatefulProperties()
{
  // compute initial orientation of the beam for calculating initial rotation matrix
  const std::vector<RealGradient> * orientation =
      &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
  RealGradient x_orientation = (*orientation)[0];
  x_orientation /= x_orientation.norm();

  RealGradient y_orientation = getParam<RealGradient>("y_orientation");
  Real sum = x_orientation(0) * y_orientation(0) + x_orientation(1) * y_orientation(1) +
             x_orientation(2) * y_orientation(2);

  if (abs(sum) > 1e-4)
    mooseError("Error in ComputeIncrementalBeamStrain: y_orientation should be perpendicular to "
               "the axis of the beam.");

  // Calculate z orientation as a cross product of the x and y orientations
  RealGradient z_orientation(3, 0.0);
  z_orientation(0) = (x_orientation(1) * y_orientation(2) - x_orientation(2) * y_orientation(1));
  z_orientation(1) = (x_orientation(2) * y_orientation(0) - x_orientation(0) * y_orientation(2));
  z_orientation(2) = (x_orientation(0) * y_orientation(1) - x_orientation(1) * y_orientation(0));

  // Rotation matrix from global to original beam local configuration
  _original_local_config[_qp](0, 0) = x_orientation(0);
  _original_local_config[_qp](0, 1) = x_orientation(1);
  _original_local_config[_qp](0, 2) = x_orientation(2);
  _original_local_config[_qp](1, 0) = y_orientation(0);
  _original_local_config[_qp](1, 1) = y_orientation(1);
  _original_local_config[_qp](1, 2) = y_orientation(2);
  _original_local_config[_qp](2, 0) = z_orientation(0);
  _original_local_config[_qp](2, 1) = z_orientation(1);
  _original_local_config[_qp](2, 2) = z_orientation(2);

  // calculate original length of a beam element
  // fetch the two end nodes for current element
  std::vector<Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->get_node(i));

  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _original_length[_qp] = dxyz.norm();

  // For small rotation problems, the rotation matrix is essentially the transformation from the
  // global to original beam local configuration and is never updated
  _total_rotation[_qp] = _original_local_config[_qp];

  _K11[0].zero();
  _K21_cross[0].zero();
  _K21[0].zero();
  _K22[0].zero();
  _K22_cross[0].zero();
}

void
ComputeIncrementalBeamStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpStrain();

  if (_fe_problem.currentlyComputingJacobian())
    computeJacobian();
}

void
ComputeIncrementalBeamStrain::computeQpStrain()
{

  // fetch the two end nodes for _current_elem
  std::vector<Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->get_node(i));

  // Fetch the solution for the two end nodes at time t
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();
  const NumericVector<Number> & sol_old = nonlinear_sys.solutionOld();

  RealVectorValue disp0(3, 0.0), disp1(3, 0.0);
  RealVectorValue rot0(3, 0.0), rot1(3, 0.0);
  RealVectorValue delta_disp(3, 0.0);
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    disp0(i) = sol(node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0)) -
               sol_old(node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0));
    disp1(i) = sol(node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0)) -
               sol_old(node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0));
    rot0(i) = sol(node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0)) -
              sol_old(node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0));
    rot1(i) = sol(node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0)) -
              sol_old(node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0));
  }

  // Rotate the gradient of displacements and rotations at t+delta t from global coordinate
  // frame to beam local coordinate frame
  RealVectorValue grad_disp_0(1.0 / _original_length[0] * (disp1 - disp0));
  RealVectorValue grad_rot_0(1.0 / _original_length[0] * (rot1 - rot0));
  RealVectorValue avg_rot(
      0.5 * (rot0(0) + rot1(0)), 0.5 * (rot0(1) + rot1(1)), 0.5 * (rot0(2) + rot1(2)));

  _grad_disp_0_local_t = _total_rotation[0] * grad_disp_0;
  _grad_rot_0_local_t = _total_rotation[0] * grad_rot_0;
  _avg_rot_local_t = _total_rotation[0] * avg_rot;

  // displacement at any location on beam in local coordinate system at t
  // u_1 = u_n1 - rot_3 * y + rot_2 * z
  // u_2 = u_n2 - rot_1 * z
  // u_3 = u_n3 + rot_1 * y
  // where u_n1, u_n2, u_n3 are displacements at neutral axis

  // small strain
  // e_11 = u_1,1 = u_n1, 1 - rot_3, 1 * y + rot_2, 1 * z
  // e_12 = 2 * 0.5 * (u_1,2 + u_2,1) = (- rot_3 + u_n2,1 - rot_1,1 * z)
  // e_13 = 2 * 0.5 * (u_1,3 + u_3,1) = (rot_2 + u_n3,1 + rot_1,1 * y)

  // axial and shearing strains at each qp along the length of the beam
  _disp_strain_increment[_qp](0) = _grad_disp_0_local_t(0) * _area[_qp] -
                                   _grad_rot_0_local_t(2) * _Ay[_qp] +
                                   _grad_rot_0_local_t(1) * _Az[_qp];
  _disp_strain_increment[_qp](1) = -_avg_rot_local_t(2) * _area[_qp] +
                                   _grad_disp_0_local_t(1) * _area[_qp] -
                                   _grad_rot_0_local_t(0) * _Az[_qp];
  _disp_strain_increment[_qp](2) = _avg_rot_local_t(1) * _area[_qp] +
                                   _grad_disp_0_local_t(2) * _area[_qp] +
                                   _grad_rot_0_local_t(0) * _Ay[_qp];

  // rotational strains at each qp along the length of the beam
  // rot_strain_1 = integral(e_13 * y - e_12 * z) dA
  // rot_strain_2 = integral(e_11 * z) dA
  // rot_strain_3 = integral(e_11 * y) dA
  // J is the product moment of inertia which is zero for most cross-sections so it is assumed to be
  // zero for this analysis
  Real J = 0;
  _rot_strain_increment[_qp](0) =
      _avg_rot_local_t(1) * _Ay[_qp] + _grad_disp_0_local_t(2) * _Ay[_qp] +
      _grad_rot_0_local_t(0) * _Iy[_qp] + _avg_rot_local_t(2) * _Az[_qp] -
      _grad_disp_0_local_t(1) * _Az[_qp] + _grad_rot_0_local_t(0) * _Iz[_qp];
  _rot_strain_increment[_qp](1) = _grad_disp_0_local_t(0) * _Az[_qp] - _grad_rot_0_local_t(2) * J +
                                  _grad_rot_0_local_t(1) * _Iz[_qp];
  _rot_strain_increment[_qp](2) = _grad_disp_0_local_t(0) * _Ay[_qp] -
                                  _grad_rot_0_local_t(2) * _Iy[_qp] + _grad_rot_0_local_t(1) * J;

  if (_large_strain)
  {
    _disp_strain_increment[_qp](0) +=
        0.5 *
        ((Utility::pow<2>(_grad_disp_0_local_t(0)) + Utility::pow<2>(_grad_disp_0_local_t(1)) +
          Utility::pow<2>(_grad_disp_0_local_t(2))) *
             _area[_qp] +
         Utility::pow<2>(_grad_rot_0_local_t(2)) * _Iy[_qp] +
         Utility::pow<2>(_grad_rot_0_local_t(1)) * _Iz[_qp] +
         Utility::pow<2>(_grad_rot_0_local_t(0)) * (_Iy[_qp] + _Iz[_qp]));
    _disp_strain_increment[_qp](1) += (-_avg_rot_local_t(2) * _grad_disp_0_local_t(0) +
                                       _avg_rot_local_t(0) * _grad_disp_0_local_t(2)) *
                                      _area[_qp];
    _disp_strain_increment[_qp](2) += (_avg_rot_local_t(1) * _grad_disp_0_local_t(0) -
                                       _avg_rot_local_t(0) * _grad_disp_0_local_t(1)) *
                                      _area[_qp];

    _rot_strain_increment[_qp](0) += -_avg_rot_local_t(1) * _grad_rot_0_local_t(2) * _Iy[_qp] +
                                     _avg_rot_local_t(2) * _grad_rot_0_local_t(1) * _Iz[_qp];
    _rot_strain_increment[_qp](1) += (_grad_disp_0_local_t(0) * _grad_rot_0_local_t(1) -
                                      _grad_disp_0_local_t(1) * _grad_rot_0_local_t(0)) *
                                     _Iz[_qp];
    _rot_strain_increment[_qp](2) += (_grad_disp_0_local_t(2) * _grad_rot_0_local_t(0) -
                                      _grad_disp_0_local_t(0) * _grad_rot_0_local_t(2)) *
                                     _Iy[_qp];
  }
}

void
ComputeIncrementalBeamStrain::computeJacobian()
{
  Real youngs_modulus = _material_stiffness[0](0);
  Real shear_modulus = _material_stiffness[0](1);

  Real A_avg = (_area[0] + _area[1]) / 2.0;
  Real Iy_avg = (_Iy[0] + _Iy[1]) / 2.0;
  Real Iz_avg = (_Iz[0] + _Iz[1]) / 2.0;

  // K = |K11 K12|
  //     |K21 K22|

  // relation between translational displacements at node 0 and translational forces at node 0
  RankTwoTensor K11_local;
  K11_local.zero();
  K11_local(0, 0) = youngs_modulus * A_avg / _original_length[0];
  K11_local(1, 1) = shear_modulus * A_avg / _original_length[0];
  K11_local(2, 2) = shear_modulus * A_avg / _original_length[0];
  _K11[0] = _total_rotation[0].transpose() * K11_local * _total_rotation[0];

  // relation between displacements at node 0 and rotational moments at node 0
  RankTwoTensor K21_local;
  K21_local.zero();
  K21_local(2, 1) = shear_modulus * A_avg * 0.5;
  K21_local(1, 2) = -shear_modulus * A_avg * 0.5;
  _K21[0] = _total_rotation[0].transpose() * K21_local * _total_rotation[0];

  // relation between rotations at node 0 and rotational moments at node 0
  RankTwoTensor K22_local;
  K22_local.zero();
  K22_local(0, 0) = shear_modulus * (Iy_avg + Iz_avg) / _original_length[0];
  K22_local(1, 1) = youngs_modulus * Iz_avg / _original_length[0] +
                    shear_modulus * A_avg * _original_length[0] / 4.0;
  K22_local(2, 2) = youngs_modulus * Iy_avg / _original_length[0] +
                    shear_modulus * A_avg * _original_length[0] / 4.0;
  _K22[0] = _total_rotation[0].transpose() * K22_local * _total_rotation[0];

  // relation between rotations at node 0 and rotational moments at node 1
  RankTwoTensor K22_local_cross = -K22_local;
  K22_local_cross(1, 1) += 2.0 * shear_modulus * A_avg * _original_length[0] / 4.0;
  K22_local_cross(2, 2) += 2.0 * shear_modulus * A_avg * _original_length[0] / 4.0;
  _K22_cross[0] = _total_rotation[0].transpose() * K22_local_cross * _total_rotation[0];

  // relation between displacements at node 0 and rotational moments at node 1
  _K21_cross[0] = -_K21[0];

  // stiffness matrix for large strain
  if (_large_strain)
  {
    // k1_large is the stiffness matrix obtained from sigma_xx * d(epsilon_xx)
    RankTwoTensor k1_large_11;
    // row 1
    k1_large_11(0, 0) = Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * (Iy_avg + Iz_avg);
    k1_large_11(1, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) -
                        1.0 / 3.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg;
    k1_large_11(2, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) -
                        1.0 / 3.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg;

    // row 2
    k1_large_11(0, 1) = k1_large_11(1, 0);
    k1_large_11(1, 1) = Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iy_avg;
    k1_large_11(2, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_disp_0_local_t(2);

    // row 3
    k1_large_11(0, 2) = k1_large_11(2, 0);
    k1_large_11(1, 2) = k1_large_11(2, 1);
    k1_large_11(2, 2) = Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iz_avg;

    k1_large_11 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k1_large_21;
    // row 1
    k1_large_21(0, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * (Iy_avg + Iz_avg) -
                        1.0 / 3.0 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(2);
    k1_large_21(1, 0) = 1.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(0) * Iz_avg;
    k1_large_21(2, 0) = 1.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(0) * Iy_avg;

    // row 2
    k1_large_21(0, 1) = k1_large_21(1, 0);
    k1_large_21(1, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * Iz_avg;
    k1_large_21(2, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(2) * Iy_avg;

    // row 3
    k1_large_21(0, 2) = k1_large_21(2, 0);
    k1_large_21(1, 2) = k1_large_21(2, 1);
    k1_large_21(2, 2) = 0.5 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(2) * Iy_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * Iy_avg;
    k1_large_21 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k1_large_22;
    // row 1
    k1_large_22(0, 0) =
        Utility::pow<2>(_grad_rot_0_local_t(0)) * Utility::pow<2>(Iy_avg + Iz_avg) +
        1.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iz_avg +
        1.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iy_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * (Iy_avg + Iz_avg) +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iz_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iy_avg +
        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * (Iy_avg * Iz_avg + Iy_avg * Iy_avg) +
        0.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * (Iz_avg * Iz_avg + Iy_avg * Iz_avg);
    k1_large_22(1, 0) = 0.5 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) *
                            (Iz_avg * Iz_avg + Iz_avg * Iy_avg) -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) * Iz_avg;
    k1_large_22(2, 0) = 0.5 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) *
                            (Iy_avg * Iz_avg + Iy_avg * Iy_avg) -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) * Iy_avg;

    // row 2
    k1_large_22(0, 1) = k1_large_22(1, 0);
    k1_large_22(1, 1) =
        Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg * Iz_avg +
        1.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * Iz_avg +
        1.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg * Iz_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iz_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iz_avg +
        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * (Iz_avg * Iz_avg + Iy_avg * Iz_avg);
    k1_large_22(2, 1) = 1.5 * _grad_rot_0_local_t(1) * _grad_rot_0_local_t(2) * Iy_avg * Iz_avg;

    // row 3
    k1_large_22(0, 2) = k1_large_22(2, 0);
    k1_large_22(1, 2) = k1_large_22(2, 1);
    k1_large_22(2, 2) =
        Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg * Iy_avg +
        1.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * Iy_avg +
        1.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iy_avg * Iz_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iy_avg +
        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iy_avg +
        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * (Iz_avg * Iy_avg + Iy_avg * Iy_avg);

    k1_large_22 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    // k2_large and k3_large are constributions from tau_xy * d(gamma_xy) and tau_xz * d(gamma_xz)
    // k2_large for node 1 is negative of that for node 0
    RankTwoTensor k2_large_11;
    k2_large_11.zero();
    // col 1
    k2_large_11(0, 0) =
        0.25 * Utility::pow<2>(_avg_rot_local_t(2)) + 0.25 * Utility::pow<2>(_avg_rot_local_t(1));
    k2_large_11(1, 0) = -1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(1);
    k2_large_11(2, 0) = -1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(2);

    // col 2
    k2_large_11(0, 1) = k2_large_11(1, 0);
    k2_large_11(1, 1) = 0.25 * _avg_rot_local_t(0);

    // col 3
    k2_large_11(0, 2) = k2_large_11(2, 0);
    k2_large_11(2, 2) = 0.25 * Utility::pow<2>(_avg_rot_local_t(0));

    k2_large_11 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k2_large_22;
    k2_large_22.zero();
    // col1
    k2_large_22(0, 0) = 0.25 * Utility::pow<2>(_avg_rot_local_t(0)) * (Iy_avg + Iz_avg);
    k2_large_22(1, 0) = 1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(1) * Iz_avg;
    k2_large_22(2, 0) = 1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(2) * Iy_avg;

    // col2
    k2_large_22(0, 1) = k2_large_22(1, 0);
    k2_large_22(1, 1) = 0.25 * Utility::pow<2>(_avg_rot_local_t(2)) * Iz_avg +
                        0.25 * Utility::pow<2>(_avg_rot_local_t(1)) * Iz_avg;

    // col3
    k2_large_22(0, 2) = k2_large_22(2, 0);
    k2_large_22(2, 2) = 0.25 * Utility::pow<2>(_avg_rot_local_t(2)) * Iy_avg +
                        0.25 * Utility::pow<2>(_avg_rot_local_t(1)) * Iy_avg;

    k2_large_22 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    // k3_large for node 1 is same as that for node 0
    RankTwoTensor k3_large_22;
    k3_large_22.zero();
    // col1
    k3_large_22(0, 0) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.25 * _grad_rot_0_local_t(0) * (Iy_avg + Iz_avg) +
                        0.25 * Utility::pow<2>(_grad_disp_0_local_t(1));
    k3_large_22(1, 0) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg;
    k3_large_22(2, 0) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg;

    // col2
    k3_large_22(0, 1) = k3_large_22(1, 0);
    k3_large_22(2, 2) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.25 * _grad_rot_0_local_t(2) * Iy_avg +
                        0.25 * _grad_rot_0_local_t(1) * Iz_avg;

    // col3
    k3_large_22(0, 2) = k3_large_22(2, 0);
    k3_large_22(2, 2) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.25 * _grad_rot_0_local_t(2) * Iy_avg +
                        0.25 * _grad_rot_0_local_t(1) * Iz_avg;

    k3_large_22 *= 1.0 / 16.0;

    RankTwoTensor k3_large_21;
    k3_large_21.zero();
    // col1
    k3_large_21(0, 0) = -1.0 / 6.0 * (_grad_disp_0_local_t(2) * _avg_rot_local_t(2) +
                                      _grad_disp_0_local_t(1) * _avg_rot_local_t(1));
    k3_large_21(1, 0) = 0.25 * _grad_disp_0_local_t(0) * _avg_rot_local_t(1) -
                        1.0 / 6.0 * _grad_disp_0_local_t(1) * _avg_rot_local_t(0);
    k3_large_21(2, 0) = 0.25 * _grad_disp_0_local_t(0) * _avg_rot_local_t(2) -
                        1.0 / 6.0 * _grad_disp_0_local_t(2) * _avg_rot_local_t(0);

    // col2
    k3_large_21(0, 1) = 0.25 * _grad_disp_0_local_t(1) * _avg_rot_local_t(0) -
                        1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(1);
    k3_large_21(1, 1) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(0);

    // col3
    k3_large_21(0, 2) = 0.25 * _grad_disp_0_local_t(2) * _avg_rot_local_t(0) -
                        1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(2);
    k3_large_21(2, 2) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(0);

    k3_large_21 *= 1.0 / 8.0 / _original_length[0];

    RankTwoTensor k4_large_22;
    k4_large_22.zero();
    // col 1
    k4_large_22(0, 0) = 0.25 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * (Iy_avg + Iz_avg) +
                        1.0 / 6.0 * _grad_rot_0_local_t(2) * _avg_rot_local_t(2) * Iy_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(1) * _avg_rot_local_t(1) * Iz_avg;
    k4_large_22(1, 0) = 1.0 / 6.0 * _grad_rot_0_local_t(1) * _avg_rot_local_t(0) * Iz_avg;
    k4_large_22(2, 0) = 1.0 / 6.0 * _grad_rot_0_local_t(2) * _avg_rot_local_t(0) * Iy_avg;

    // col2
    k4_large_22(0, 1) = 1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(1) * Iz_avg;
    k4_large_22(1, 1) = 0.25 * _grad_rot_0_local_t(1) * _avg_rot_local_t(1) * Iz_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * Iz_avg;
    k4_large_22(2, 1) = 0.25 * _grad_rot_0_local_t(1) * _avg_rot_local_t(2) * Iz_avg;

    // col 3
    k4_large_22(0, 2) = 1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(2) * Iy_avg;
    k4_large_22(1, 2) = 0.25 * _grad_rot_0_local_t(2) * _avg_rot_local_t(1) * Iy_avg;
    k4_large_22(2, 2) = 0.25 * _grad_rot_0_local_t(2) * _avg_rot_local_t(2) * Iy_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * Iy_avg;

    k3_large_22 += 1.0 / 8.0 / _original_length[0] * (k4_large_22 + k4_large_22.transpose());

    // Assembling final matrix
    _K11[0] += _total_rotation[0].transpose() * (k1_large_11 + k2_large_11) * _total_rotation[0];
    _K22[0] += _total_rotation[0].transpose() * (k1_large_22 + k2_large_22 + k3_large_22) *
               _total_rotation[0];
    _K21[0] += _total_rotation[0].transpose() * (k1_large_21 + k3_large_21) * _total_rotation[0];
    _K21_cross[0] +=
        _total_rotation[0].transpose() * (-k1_large_21 + k3_large_21) * _total_rotation[0];
    _K22_cross[0] += _total_rotation[0].transpose() * (-k1_large_22 - k2_large_22 + k3_large_22) *
                     _total_rotation[0];
  }
}
