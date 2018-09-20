//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RealMortarConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "AutomaticMortarGeneration.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<RealMortarConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<unsigned>("master_id", "The id of the master boundary sideset.");
  params.addRequiredParam<unsigned>("slave_id", "The id of the slave boundary sideset.");
  params.registerRelationshipManagers("AugmentSparsityOnInterface");
  params.addRequiredParam<VariableName>("primal_variable", "The primal variable");
  params.addRequiredParam<VariableName>("lm_variable", "The lagrange multiplier variable");
  return params;
}

RealMortarConstraint::RealMortarConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _dim(_mesh.dimension()),

    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _current_elem(_assembly.elem()),

    _primal_var(_subproblem.getStandardVariable(_tid, getParam<VariableName>("primal_variable"))),
    _lambda_var(_subproblem.getStandardVariable(_tid, getParam<VariableName>("lm_variable"))),

    _test_primal(_primal_var.phi()),
    _grad_test_primal(_primal_var.gradPhi()),
    _phi_primal(_primal_var.phi()),

    _slave_id(getParam<unsigned>("slave_id")),
    _master_id(getParam<unsigned>("master_id")),
    _amg(_fe_problem.getMortarInterface(std::make_pair(_slave_id, _master_id)))
{
}

void
RealMortarConstraint::computeResidual()
{
  // Get a constant reference to the mesh object.
  const MeshBase & mesh = _fe_problem.mesh().getMesh();

  // The dimension that we are running
  const unsigned int dim = mesh.mesh_dimension();

  // A reference to the DofMap object for this system.
  const DofMap & dof_map = _sys.dofMap();

  // Get a constant reference to the Finite Element type
  // for the first (and only) variable in the system.
  auto primal_var_number = _primal_var.number();
  auto lambda_var_number = _lambda_var.number();
  FEType fe_type_primal = dof_map.variable_type(primal_var_number),
         fe_type_lambda = dof_map.variable_type(lambda_var_number);

  // A FE object for assembling on lower-dimensional elements.
  UniquePtr<FEBase> fe_msm(FEBase::build(dim - 1, fe_type_primal));
  QGauss qrule_msm(dim - 1, SECOND);
  fe_msm->attach_quadrature_rule(&qrule_msm);

  // Pre-request lower-dimensional element values. Only thing needed is really the JxW values.
  const std::vector<Real> & JxW_msm = fe_msm->get_JxW();

  // A lower-dimensional FE object for the _lambda_var.
  UniquePtr<FEBase> slave_face_fe_lambda(FEBase::build(dim - 1, fe_type_lambda));
  const std::vector<std::vector<Real>> & slave_face_phi_lambda = slave_face_fe_lambda->get_phi();

  // A finite element object for the interior element associated with
  // the temperature variable on the slave side.  This is used for computing
  // stabilization terms that depend on the boundary flux from the
  // interior element.
  UniquePtr<FEBase> slave_interior_fe_primal(FEBase::build(dim, fe_type_primal));
  const std::vector<std::vector<Real>> & slave_interior_phi_primal =
      slave_interior_fe_primal->get_phi();
  const std::vector<Point> & slave_interior_xyz = slave_interior_fe_primal->get_xyz();

  // A finite element object for the interior element associated with
  // the temperature variable on the master side.  This is used for computing
  // stabilization terms that depend on the boundary flux from the
  // interior element.
  UniquePtr<FEBase> master_interior_fe_primal(FEBase::build(dim, fe_type_primal));
  const std::vector<std::vector<Real>> & master_interior_phi_primal =
      master_interior_fe_primal->get_phi();
  const std::vector<Point> & master_interior_xyz = master_interior_fe_primal->get_xyz();

  // Vector to hold lambda dofs on lower-dimensional slave side elements
  std::vector<dof_id_type> dof_indices_slave_lambda;

  // Vector to hold Temperature dofs of slave side and master side
  // interior elements.
  std::vector<dof_id_type> dof_indices_interior_slave_primal;
  std::vector<dof_id_type> dof_indices_interior_master_primal;

  DenseVector<Real> F_primal_slave;
  DenseVector<Real> F_primal_master;
  DenseVector<Real> F_lambda_slave;

  DenseVector<Real> u_primal_slave;
  DenseVector<Real> u_primal_master;
  DenseVector<Real> u_lambda_slave;

  const NumericVector<Real> & current_solution = *_sys.currentSolution();

  // Array to hold custom quadrature point locations on the slave and master sides
  std::vector<Point> custom_xi1_pts, custom_xi2_pts;

  libMesh::out << "About to loop over " << _amg.mortar_segment_mesh.n_elem()
               << " mortar mesh segments." << std::endl;

  for (MeshBase::const_element_iterator
           el = _amg.mortar_segment_mesh.active_local_elements_begin(),
           end_el = _amg.mortar_segment_mesh.active_local_elements_end();
       el != end_el;
       ++el)
  {
    // Note: this is a mortar segment mesh Elem, *not* an Elem
    // from the original mesh or the side of an Elem from the
    // original mesh.
    const Elem * msm_elem = *el;

    // We may eventually allow zero-length segments in the
    // MSM. They are OK connectivity-wise, and they don't
    // contribute to the mortar segment integrals, but we don't
    // want to do reinit() on them or there will be a negative
    // Jacobian error.
    Real elem_volume = msm_elem->volume();

    // libMesh::out << "Computing element integral contributions for mortar segment mesh Elem " <<
    // msm_elem->id()
    //              << " which has length " << elem_volume
    //              << std::endl;

    if (elem_volume < TOLERANCE)
      continue;

    // Get a reference to the MortarSegmentInfo for this Elem.
    const MortarSegmentInfo & msinfo = _amg.msm_elem_to_info.at(msm_elem);

    // There may be no contribution from the master side if it is not "in contact".
    bool has_slave = msinfo.slave_elem ? true : false, has_master = msinfo.has_master();

    // Print the slave and master (if available) elements associated
    // with this mortar segment. Note: these are lower-dimensional
    // elements from the *surface* meshes, they will not have the
    // DOFs that we actually need on them.
    if (has_slave)
    {
      // libMesh::out << "Mortar segment is associated with slave mesh Elem " <<
      // msinfo.slave_elem->id() << std::endl; libMesh::out << "xi^(1) = [" << msinfo.xi1_a << ", "
      // << msinfo.xi1_b << "]" << std::endl;
    }
    else
      libmesh_error_msg("Error, mortar segment has no slave element associated with it!");

    // if (has_master)
    //   {
    //     libMesh::out << "Mortar segment is associated with master mesh Elem " <<
    //     msinfo.master_elem->id() << std::endl; libMesh::out << "xi^(2) = [" << msinfo.xi2_a << ",
    //     " << msinfo.xi2_b << "]" << std::endl;
    //   }

    // Need to figure out which Elems that have system DOFs these correspond to.

    // Pointer to the interior parent.
    const Elem * slave_ip = msinfo.slave_elem->interior_parent();

    // Look up which side of the interior parent we are.
    unsigned int slave_side_id = _amg.lower_elem_to_side_id.at(msinfo.slave_elem);

    // libMesh::out << "slave mesh elem has interior parent " << slave_ip->id() << std::endl;
    // libMesh::out << "slave mesh elem is side " << slave_side_id << " of this parent." <<
    // std::endl;

    // The lower-dimensional slave side element associated with this
    // mortar segment is simply msinfo.slave_elem.
    const Elem * slave_face_elem = msinfo.slave_elem;

    // Print the lower-dimensional element id. We can now ask for DOFs specifically for this
    // element. libMesh::out << "Lower-dimensional element id: " << slave_face_elem->id() <<
    // std::endl;

    // Get the temperature and lambda dof indices for the slave element side.
    dof_map.dof_indices(slave_face_elem, dof_indices_slave_lambda, lambda_var_number);

    // Get temperature dof indices associated with the interior slave Elem.
    dof_map.dof_indices(slave_ip, dof_indices_interior_slave_primal, primal_var_number);

    // Print the degree of freedom indices for the lambda variable on this lower-dimensional
    // element. libMesh::out << "lambda DOFs for the lower-dimensional slave element: "; for
    // (dof_id_type i=0; i<dof_indices_slave_lambda.size(); ++i)
    //   libMesh::out << dof_indices_slave_lambda[i] << " ";
    // libMesh::out << std::endl;

    // Print the degree of freedom indices for the T variable on the slave interior element.
    // libMesh::out << "T DOFs for the slave interior element: ";
    // for (dof_id_type i=0; i<dof_indices_interior_slave_primal.size(); ++i)
    //   libMesh::out << dof_indices_interior_slave_primal[i] << " ";
    // libMesh::out << std::endl;

    // These only get initialized if there is a master Elem associated to this segment.
    const Elem * master_ip = libmesh_nullptr;
    unsigned int master_side_id = libMesh::invalid_uint;
    Real h_master = 0.;

    if (has_master)
    {
      // Set the master interior parent and side ids.
      master_ip = msinfo.master_elem->interior_parent();
      master_side_id = _amg.lower_elem_to_side_id.at(msinfo.master_elem);

      // Store the length of the edge element for use in stabilization terms.
      h_master = msinfo.master_elem->volume();

      // libMesh::out << "master mesh elem has interior parent " << master_ip->id() << std::endl;
      // libMesh::out << "master mesh elem is side " << master_side_id << " of this parent." <<
      // std::endl;

      // Get temperature dof indices associated with the interior master Elem.
      dof_map.dof_indices(master_ip, dof_indices_interior_master_primal, primal_var_number);

      // Print the degree of freedom indices for the T variable on the slave interior element.
      // libMesh::out << "T DOFs for the master interior element: ";
      // for (dof_id_type i=0; i<dof_indices_interior_master_primal.size(); ++i)
      //   libMesh::out << dof_indices_interior_master_primal[i] << " ";
      // libMesh::out << std::endl;
    }

    // Re-compute FE data on the mortar segment Elem (JxW_msm is the only thing we actually use.)
    fe_msm->reinit(msm_elem);

    F_primal_slave.resize(dof_indices_interior_slave_primal.size());
    F_primal_master.resize(dof_indices_interior_master_primal.size());
    F_lambda_slave.resize(dof_indices_slave_lambda.size());

    u_primal_slave.resize(qrule_msm.n_points());
    u_primal_master.resize(qrule_msm.n_points());
    u_lambda_slave.resize(qrule_msm.n_points());

    {
      custom_xi1_pts.resize(qrule_msm.n_points());
      // libMesh::out << "xi^(1) points: ";
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      {
        Real eta = qrule_msm.qp(qp)(0);
        Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
        custom_xi1_pts[qp] = xi1_eta;
        // libMesh::out << xi1_eta << " ";
      }
      // libMesh::out << std::endl;

      // When you don't provide a weights array, the FE object uses
      // a dummy rule with all 1's. You should probably never use
      // the resulting JxW values for anything important.
      slave_face_fe_lambda->reinit(slave_face_elem, &custom_xi1_pts);

      // Reinit the slave interior FE on the side in question at the custom qps.
      slave_interior_fe_primal->reinit(slave_ip, slave_side_id, TOLERANCE, &custom_xi1_pts);

      // libMesh::out << "Slave element lambda phi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_face_phi_lambda.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " << std::setw(12) <<
      //         slave_face_phi_lambda[i][qp] << " ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element normal vectors:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << std::setw(12) << slave_interior_normals[qp] <<
      //   std::endl;
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element dphi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_interior_dphi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         slave_interior_dphi_primal[i][qp] << "   &    ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element phi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_interior_phi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         slave_interior_phi_primal[i][qp]
      //         << " ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element xyz(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << slave_interior_xyz[qp] << " ";
      // libMesh::out << std::endl;
    }

    if (has_master)
    {
      custom_xi2_pts.resize(qrule_msm.n_points());
      // libMesh::out << "xi^(2) points: ";
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      {
        Real eta = qrule_msm.qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
        // libMesh::out << xi2_eta << " ";
      }
      // libMesh::out << std::endl;

      // Reinit the master interior FE on the side in question at the custom qps.
      master_interior_fe_primal->reinit(master_ip, master_side_id, TOLERANCE, &custom_xi2_pts);

      // libMesh::out << "Master interior element normal vectors:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << std::setw(12) << master_interior_normals[qp] <<
      //   std::endl;
      // libMesh::out << std::endl;

      // libMesh::out << "Master interior element dphi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<master_interior_dphi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         master_interior_dphi_primal[i][qp] << "   &    ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Master interior element xyz(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << master_interior_xyz[qp] << " ";
      // libMesh::out << std::endl;
    }

    // For the gap conductance problem, compute the gap heat
    // transfer coefficient, which is proportional to (k/g) where k
    // is the thermal conductivity of the air in the gap, and g is
    // the distance between the surfaces.
    std::vector<Real> heat_transfer_coeff(slave_interior_xyz.size());
    if (has_master)
    {
      for (unsigned int qp = 0; qp < heat_transfer_coeff.size(); ++qp)
      {
        Real gap = (slave_interior_xyz[qp] - master_interior_xyz[qp]).norm();
        // libMesh::out << "gap = " << gap << std::endl;

        // Avoid division by zero. We currently only support strictly nonzero gap sizes.
        if (gap < TOLERANCE * TOLERANCE)
          libmesh_error_msg("Error: gap "
                            << gap
                            << " is approximately zero, the gap conductance approach will "
                               "fail.");

        // TODO: Currently the numerator is a fixed constant, but it should be specifiable by the
        // user.
        heat_transfer_coeff[qp] = 0.03 / gap;

        // libMesh::out << "heat_transfer_coeff[" << qp << "] = " << heat_transfer_coeff[qp] <<
        // std::endl;
      }
    }

    for (unsigned int qp = 0; qp < qrule_msm.n_points(); ++qp)
    {
      for (unsigned int i = 0; i < dof_indices_slave_lambda.size(); ++i)
      {
        auto idx = dof_indices_slave_lambda[i];
        auto soln_local = current_solution(idx);

        u_lambda_slave(qp) += soln_local * slave_face_phi_lambda[i][qp];
      }
      for (unsigned int i = 0; i < dof_indices_interior_slave_primal.size(); ++i)
      {
        auto idx = dof_indices_interior_slave_primal[i];
        auto soln_local = current_solution(idx);

        u_primal_slave(qp) += soln_local * slave_interior_phi_primal[i][qp];
      }
      for (unsigned int i = 0; i < dof_indices_interior_master_primal.size(); ++i)
      {
        auto idx = dof_indices_interior_master_primal[i];
        auto soln_local = current_solution(idx);

        u_primal_master(qp) += soln_local * master_interior_phi_primal[i][qp];
      }
    }

    // Compute slave/slave contribution to primal equation.
    for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      for (unsigned int i = 0; i < dof_indices_interior_slave_primal.size(); ++i)
        F_primal_slave(i) += JxW_msm[qp] * slave_interior_phi_primal[i][qp] * u_lambda_slave(qp);

    // In the continuity constrained case, the slave/slave
    // contribution to the LM equation is just the transpose of the
    // primal contribution. In the gap conductance case, the heat
    // transfer coefficient shows up, and there is a minus sign.
    for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      for (unsigned int i = 0; i < dof_indices_slave_lambda.size(); ++i)
        F_lambda_slave(i) += JxW_msm[qp] * slave_face_phi_lambda[i][qp] *
                             (u_lambda_slave(qp) -
                              heat_transfer_coeff[qp] *
                                  (u_primal_slave(qp) - (has_master ? u_primal_master(qp) : 0)));

    // Compute master/slave contributions to primal equation.
    if (has_master)
    {
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
        for (unsigned int i = 0; i < dof_indices_interior_master_primal.size(); ++i)
          F_primal_master(i) +=
              -JxW_msm[qp] * master_interior_phi_primal[i][qp] * u_lambda_slave(qp);
    }

    _sys.getVector(_sys.residualVectorTag()).add_vector(F_lambda_slave, dof_indices_slave_lambda);
    _sys.getVector(_sys.residualVectorTag())
        .add_vector(F_primal_slave, dof_indices_interior_slave_primal);
    _sys.getVector(_sys.residualVectorTag())
        .add_vector(F_primal_master, dof_indices_interior_master_primal);
  }
}

void
RealMortarConstraint::computeJacobian()
{
  // Get a constant reference to the mesh object.
  const MeshBase & mesh = _fe_problem.mesh().getMesh();

  // The dimension that we are running
  const unsigned int dim = mesh.mesh_dimension();

  // A reference to the DofMap object for this system.
  const DofMap & dof_map = _sys.dofMap();

  // Get a constant reference to the Finite Element type
  // for the first (and only) variable in the system.
  auto primal_var_number = _primal_var.number();
  auto lambda_var_number = _lambda_var.number();
  FEType fe_type_primal = dof_map.variable_type(primal_var_number),
         fe_type_lambda = dof_map.variable_type(lambda_var_number);

  // A FE object for assembling on lower-dimensional elements.
  UniquePtr<FEBase> fe_msm(FEBase::build(dim - 1, fe_type_primal));
  QGauss qrule_msm(dim - 1, SECOND);
  fe_msm->attach_quadrature_rule(&qrule_msm);

  // Pre-request lower-dimensional element values. Only thing needed is really the JxW values.
  const std::vector<Real> & JxW_msm = fe_msm->get_JxW();

  // A lower-dimensional FE object for the _lambda_var.
  UniquePtr<FEBase> slave_face_fe_lambda(FEBase::build(dim - 1, fe_type_lambda));
  const std::vector<std::vector<Real>> & slave_face_phi_lambda = slave_face_fe_lambda->get_phi();

  // A finite element object for the interior element associated with
  // the temperature variable on the slave side.  This is used for computing
  // stabilization terms that depend on the boundary flux from the
  // interior element.
  UniquePtr<FEBase> slave_interior_fe_primal(FEBase::build(dim, fe_type_primal));
  const std::vector<std::vector<Real>> & slave_interior_phi_primal =
      slave_interior_fe_primal->get_phi();
  const std::vector<Point> & slave_interior_xyz = slave_interior_fe_primal->get_xyz();

  // A finite element object for the interior element associated with
  // the temperature variable on the master side.  This is used for computing
  // stabilization terms that depend on the boundary flux from the
  // interior element.
  UniquePtr<FEBase> master_interior_fe_primal(FEBase::build(dim, fe_type_primal));
  const std::vector<std::vector<Real>> & master_interior_phi_primal =
      master_interior_fe_primal->get_phi();
  const std::vector<Point> & master_interior_xyz = master_interior_fe_primal->get_xyz();

  // Vector to hold lambda dofs on lower-dimensional slave side elements
  std::vector<dof_id_type> dof_indices_slave_lambda;

  // Vector to hold Temperature dofs of slave side and master side
  // interior elements.
  std::vector<dof_id_type> dof_indices_interior_slave_primal;
  std::vector<dof_id_type> dof_indices_interior_master_primal;

  // Place to accumulate slave/slave and master/slave mortar element stiffness matrices and their
  // transposes.
  DenseMatrix<Real> K_primal_slave_lambda_slave;
  DenseMatrix<Real> K_primal_master_lambda_slave;

  DenseMatrix<Real> K_lambda_slave_lambda_slave;
  DenseMatrix<Real> K_lambda_slave_primal_slave;
  DenseMatrix<Real> K_lambda_slave_primal_master;

  // Array to hold custom quadrature point locations on the slave and master sides
  std::vector<Point> custom_xi1_pts, custom_xi2_pts;

  libMesh::out << "About to loop over " << _amg.mortar_segment_mesh.n_elem()
               << " mortar mesh segments." << std::endl;

  for (MeshBase::const_element_iterator
           el = _amg.mortar_segment_mesh.active_local_elements_begin(),
           end_el = _amg.mortar_segment_mesh.active_local_elements_end();
       el != end_el;
       ++el)
  {
    // Note: this is a mortar segment mesh Elem, *not* an Elem
    // from the original mesh or the side of an Elem from the
    // original mesh.
    const Elem * msm_elem = *el;

    // We may eventually allow zero-length segments in the
    // MSM. They are OK connectivity-wise, and they don't
    // contribute to the mortar segment integrals, but we don't
    // want to do reinit() on them or there will be a negative
    // Jacobian error.
    Real elem_volume = msm_elem->volume();

    // libMesh::out << "Computing element integral contributions for mortar segment mesh Elem " <<
    // msm_elem->id()
    //              << " which has length " << elem_volume
    //              << std::endl;

    if (elem_volume < TOLERANCE)
      continue;

    // Get a reference to the MortarSegmentInfo for this Elem.
    const MortarSegmentInfo & msinfo = _amg.msm_elem_to_info.at(msm_elem);

    // There may be no contribution from the master side if it is not "in contact".
    bool has_slave = msinfo.slave_elem ? true : false, has_master = msinfo.has_master();

    // Print the slave and master (if available) elements associated
    // with this mortar segment. Note: these are lower-dimensional
    // elements from the *surface* meshes, they will not have the
    // DOFs that we actually need on them.
    if (has_slave)
    {
      // libMesh::out << "Mortar segment is associated with slave mesh Elem " <<
      // msinfo.slave_elem->id() << std::endl; libMesh::out << "xi^(1) = [" << msinfo.xi1_a << ", "
      // << msinfo.xi1_b << "]" << std::endl;
    }
    else
      libmesh_error_msg("Error, mortar segment has no slave element associated with it!");

    // if (has_master)
    //   {
    //     libMesh::out << "Mortar segment is associated with master mesh Elem " <<
    //     msinfo.master_elem->id() << std::endl; libMesh::out << "xi^(2) = [" << msinfo.xi2_a << ",
    //     " << msinfo.xi2_b << "]" << std::endl;
    //   }

    // Need to figure out which Elems that have system DOFs these correspond to.

    // Pointer to the interior parent.
    const Elem * slave_ip = msinfo.slave_elem->interior_parent();

    // Look up which side of the interior parent we are.
    unsigned int slave_side_id = _amg.lower_elem_to_side_id.at(msinfo.slave_elem);

    // libMesh::out << "slave mesh elem has interior parent " << slave_ip->id() << std::endl;
    // libMesh::out << "slave mesh elem is side " << slave_side_id << " of this parent." <<
    // std::endl;

    // The lower-dimensional slave side element associated with this
    // mortar segment is simply msinfo.slave_elem.
    const Elem * slave_face_elem = msinfo.slave_elem;

    // Print the lower-dimensional element id. We can now ask for DOFs specifically for this
    // element. libMesh::out << "Lower-dimensional element id: " << slave_face_elem->id() <<
    // std::endl;

    // Get the temperature and lambda dof indices for the slave element side.
    dof_map.dof_indices(slave_face_elem, dof_indices_slave_lambda, lambda_var_number);

    // Get temperature dof indices associated with the interior slave Elem.
    dof_map.dof_indices(slave_ip, dof_indices_interior_slave_primal, primal_var_number);

    // Print the degree of freedom indices for the lambda variable on this lower-dimensional
    // element. libMesh::out << "lambda DOFs for the lower-dimensional slave element: "; for
    // (dof_id_type i=0; i<dof_indices_slave_lambda.size(); ++i)
    //   libMesh::out << dof_indices_slave_lambda[i] << " ";
    // libMesh::out << std::endl;

    // Print the degree of freedom indices for the T variable on the slave interior element.
    // libMesh::out << "T DOFs for the slave interior element: ";
    // for (dof_id_type i=0; i<dof_indices_interior_slave_primal.size(); ++i)
    //   libMesh::out << dof_indices_interior_slave_primal[i] << " ";
    // libMesh::out << std::endl;

    // These only get initialized if there is a master Elem associated to this segment.
    const Elem * master_ip = libmesh_nullptr;
    unsigned int master_side_id = libMesh::invalid_uint;
    Real h_master = 0.;

    if (has_master)
    {
      // Set the master interior parent and side ids.
      master_ip = msinfo.master_elem->interior_parent();
      master_side_id = _amg.lower_elem_to_side_id.at(msinfo.master_elem);

      // Store the length of the edge element for use in stabilization terms.
      h_master = msinfo.master_elem->volume();

      // libMesh::out << "master mesh elem has interior parent " << master_ip->id() << std::endl;
      // libMesh::out << "master mesh elem is side " << master_side_id << " of this parent." <<
      // std::endl;

      // Get temperature dof indices associated with the interior master Elem.
      dof_map.dof_indices(master_ip, dof_indices_interior_master_primal, primal_var_number);

      // Print the degree of freedom indices for the T variable on the slave interior element.
      // libMesh::out << "T DOFs for the master interior element: ";
      // for (dof_id_type i=0; i<dof_indices_interior_master_primal.size(); ++i)
      //   libMesh::out << dof_indices_interior_master_primal[i] << " ";
      // libMesh::out << std::endl;
    }

    // Re-compute FE data on the mortar segment Elem (JxW_msm is the only thing we actually use.)
    fe_msm->reinit(msm_elem);

    K_primal_slave_lambda_slave.resize(dof_indices_interior_slave_primal.size(),
                                       dof_indices_slave_lambda.size());
    K_primal_master_lambda_slave(dof_indices_interior_master_primal.size(),
                                 dof_indices_slave_lambda.size());

    K_lambda_slave_lambda_slave.resize(dof_indices_slave_lambda.size(),
                                       dof_indices_slave_lambda.size());
    K_lambda_slave_primal_slave.resize(dof_indices_slave_lambda.size(),
                                       dof_indices_interior_slave_primal.size());
    K_lambda_slave_primal_master.resize(dof_indices_slave_lambda.size(),
                                        dof_indices_interior_master_primal.size());

    {
      custom_xi1_pts.resize(qrule_msm.n_points());
      // libMesh::out << "xi^(1) points: ";
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      {
        Real eta = qrule_msm.qp(qp)(0);
        Real xi1_eta = 0.5 * (1 - eta) * msinfo.xi1_a + 0.5 * (1 + eta) * msinfo.xi1_b;
        custom_xi1_pts[qp] = xi1_eta;
        // libMesh::out << xi1_eta << " ";
      }
      // libMesh::out << std::endl;

      // When you don't provide a weights array, the FE object uses
      // a dummy rule with all 1's. You should probably never use
      // the resulting JxW values for anything important.
      slave_face_fe_lambda->reinit(slave_face_elem, &custom_xi1_pts);

      // Reinit the slave interior FE on the side in question at the custom qps.
      slave_interior_fe_primal->reinit(slave_ip, slave_side_id, TOLERANCE, &custom_xi1_pts);

      // libMesh::out << "Slave element lambda phi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_face_phi_lambda.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " << std::setw(12) <<
      //         slave_face_phi_lambda[i][qp] << " ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element normal vectors:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << std::setw(12) << slave_interior_normals[qp] <<
      //   std::endl;
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element dphi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_interior_dphi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         slave_interior_dphi_primal[i][qp] << "   &    ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element phi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<slave_interior_phi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         slave_interior_phi_primal[i][qp]
      //         << " ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Slave interior element xyz(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << slave_interior_xyz[qp] << " ";
      // libMesh::out << std::endl;
    }

    if (has_master)
    {
      custom_xi2_pts.resize(qrule_msm.n_points());
      // libMesh::out << "xi^(2) points: ";
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      {
        Real eta = qrule_msm.qp(qp)(0);
        Real xi2_eta = 0.5 * (1 - eta) * msinfo.xi2_a + 0.5 * (1 + eta) * msinfo.xi2_b;
        custom_xi2_pts[qp] = xi2_eta;
        // libMesh::out << xi2_eta << " ";
      }
      // libMesh::out << std::endl;

      // Reinit the master interior FE on the side in question at the custom qps.
      master_interior_fe_primal->reinit(master_ip, master_side_id, TOLERANCE, &custom_xi2_pts);

      // libMesh::out << "Master interior element normal vectors:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << std::setw(12) << master_interior_normals[qp] <<
      //   std::endl;
      // libMesh::out << std::endl;

      // libMesh::out << "Master interior element dphi_i(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   {
      //     for (std::size_t i=0; i<master_interior_dphi_primal.size(); ++i)
      //       {
      //         libMesh::out << "[i=" << i << "][qp=" << qp << "]: " <<
      //         master_interior_dphi_primal[i][qp] << "   &    ";
      //       }
      //     libMesh::out << std::endl;
      //   }
      // libMesh::out << std::endl;

      // libMesh::out << "Master interior element xyz(x_qp) values:" << std::endl;
      // for (unsigned int qp=0; qp<qrule_msm.n_points(); qp++)
      //   libMesh::out << "[qp=" << qp << "]: " << master_interior_xyz[qp] << " ";
      // libMesh::out << std::endl;
    }

    // For the gap conductance problem, compute the gap heat
    // transfer coefficient, which is proportional to (k/g) where k
    // is the thermal conductivity of the air in the gap, and g is
    // the distance between the surfaces.
    std::vector<Real> heat_transfer_coeff(slave_interior_xyz.size());
    if (has_master)
    {
      for (unsigned int qp = 0; qp < heat_transfer_coeff.size(); ++qp)
      {
        Real gap = (slave_interior_xyz[qp] - master_interior_xyz[qp]).norm();
        // libMesh::out << "gap = " << gap << std::endl;

        // Avoid division by zero. We currently only support strictly nonzero gap sizes.
        if (gap < TOLERANCE * TOLERANCE)
          libmesh_error_msg("Error: gap "
                            << gap
                            << " is approximately zero, the gap conductance approach will "
                               "fail.");

        // TODO: Currently the numerator is a fixed constant, but it should be specifiable by the
        // user.
        heat_transfer_coeff[qp] = 0.03 / gap;

        // libMesh::out << "heat_transfer_coeff[" << qp << "] = " << heat_transfer_coeff[qp] <<
        // std::endl;
      }
    }

    // Compute slave/slave contribution to primal equation.
    for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      for (unsigned int i = 0; i < K_primal_slave_lambda_slave.m(); ++i)
        for (unsigned int j = 0; j < K_primal_slave_lambda_slave.n(); ++j)
          K_primal_slave_lambda_slave(i, j) +=
              JxW_msm[qp] * slave_interior_phi_primal[i][qp] * slave_face_phi_lambda[j][qp];

    // In the continuity constrained case, the slave/slave
    // contribution to the LM equation is just the transpose of the
    // primal contribution. In the gap conductance case, the heat
    // transfer coefficient shows up, and there is a minus sign.
    for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      for (unsigned int i = 0; i < K_lambda_slave_primal_slave.m(); ++i)
        for (unsigned int j = 0; j < K_lambda_slave_primal_slave.n(); ++j)
          K_lambda_slave_primal_slave(i, j) += JxW_msm[qp] * (-heat_transfer_coeff[qp]) *
                                               slave_interior_phi_primal[j][qp] *
                                               slave_face_phi_lambda[i][qp];

    // Compute the lambda/lambda contribution. If stailization is
    // turned on for the continuity constrained problem, this will
    // look like the (negative) mass matrix contribution. In the gap
    // conductance problem, it is a positive mass matrix
    // contribution. There are contributions from both the slave and
    // master side in general for the continuity constrained
    // problem, and when there is no master side Elem, h_master==0.
    for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
      for (unsigned int i = 0; i < K_lambda_slave_lambda_slave.m(); ++i)
        for (unsigned int j = 0; j < K_lambda_slave_lambda_slave.n(); ++j)
          K_lambda_slave_lambda_slave(i, j) +=
              JxW_msm[qp] * slave_face_phi_lambda[i][qp] * slave_face_phi_lambda[j][qp];

    // Compute master/slave contributions to primal equation.
    if (has_master)
    {
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
        for (unsigned int i = 0; i < K_primal_master_lambda_slave.m(); ++i)
          for (unsigned int j = 0; j < K_primal_master_lambda_slave.n(); ++j)
            K_primal_master_lambda_slave(i, j) +=
                -JxW_msm[qp] * master_interior_phi_primal[i][qp] * slave_face_phi_lambda[j][qp];

      // In the continuity constrained case, the master/slave
      // contribution to the LM equation is just the transpose of the
      // primal contribution. In the gap conductance case, the heat
      // transfer coefficient shows up, and there is a different sign.
      for (unsigned int qp = 0; qp < qrule_msm.n_points(); qp++)
        for (unsigned int i = 0; i < K_lambda_slave_primal_master.m(); ++i)
          for (unsigned int j = 0; j < K_lambda_slave_primal_master.n(); ++j)
            K_lambda_slave_primal_master(i, j) += JxW_msm[qp] * heat_transfer_coeff[qp] *
                                                  master_interior_phi_primal[j][qp] *
                                                  slave_face_phi_lambda[i][qp];
    }

    _sys.getMatrix(_sys.systemMatrixTag())
        .add_matrix(K_primal_slave_lambda_slave,
                    dof_indices_interior_slave_primal,
                    dof_indices_slave_lambda);
    _sys.getMatrix(_sys.systemMatrixTag())
        .add_matrix(K_primal_master_lambda_slave,
                    dof_indices_interior_master_primal,
                    dof_indices_slave_lambda);
    _sys.getMatrix(_sys.systemMatrixTag())
        .add_matrix(
            K_lambda_slave_lambda_slave, dof_indices_slave_lambda, dof_indices_slave_lambda);
    _sys.getMatrix(_sys.systemMatrixTag())
        .add_matrix(K_lambda_slave_primal_slave,
                    dof_indices_slave_lambda,
                    dof_indices_interior_slave_primal);
    _sys.getMatrix(_sys.systemMatrixTag())
        .add_matrix(K_lambda_slave_primal_master,
                    dof_indices_slave_lambda,
                    dof_indices_interior_master_primal);
  }
}
