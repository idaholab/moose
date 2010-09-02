/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ComputeQPSolution.h"

#include "numeric_vector.h"

void computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi)
{
  u=0;

  unsigned int phi_size = phi.size();

  //All of this stuff so that the loop will vectorize
  std::vector<Real> sol_vals(phi_size);
  std::vector<Real> phi_vals(phi_size);

  for (unsigned int i=0; i<phi_size; i++)
  {
    sol_vals[i] = soln(dof_indices[i]);
    phi_vals[i] = phi[i][qp];
  }

  for (unsigned int i=0; i<phi_size; i++)
  {
    u +=  phi_vals[i]*sol_vals[i];
  }
}

void computeQpSolutionAll(MooseArray<Real> & u, MooseArray<Real> & u_old, MooseArray<Real> & u_older,
                             MooseArray<RealGradient> &grad_u,  MooseArray<RealGradient> &grad_u_old, MooseArray<RealGradient> &grad_u_older,
                             MooseArray<RealTensor> &second_u,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp] = 0;
    u_old[qp] = 0;
    u_older[qp] = 0;

    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;

    second_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;

      second_u[qp] +=d2phi[i][qp]*soln_local;
    }

  }
}

void computeQpSolutionAll(MooseArray<Real> & u, MooseArray<Real> & u_old, MooseArray<Real> & u_older,
                             MooseArray<RealGradient> &grad_u,  MooseArray<RealGradient> &grad_u_old, MooseArray<RealGradient> &grad_u_older,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{

  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    u_old[qp]=0;
    u_older[qp] = 0;

    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;
    }

  }
}

void computeQpSolutionAll(MooseArray<Real> & u,
                                  MooseArray<RealGradient> &grad_u,
                                  MooseArray<RealTensor> &second_u,
                                  const NumericVector<Number> & soln,
                                  const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                  const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
    second_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
      second_u[qp] += d2phi[i][qp]*soln_local;
    }

  }
}


void computeQpSolutionAll(MooseArray<Real> & u,
                                  MooseArray<RealGradient> &grad_u,
                                  const NumericVector<Number> & soln,
                                  const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                  const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local = soln(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
    }

  }
}

void computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi)
{
  grad_u=0;

  unsigned int dphi_size = dphi.size();

  for (unsigned int i=0; i<dphi_size; i++)
  {
    grad_u += dphi[i][qp]*soln(dof_indices[i]);
  }
}

void computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi)
{
  second_u=0;

  unsigned int d2phi_size = d2phi.size();

  for (unsigned int i=0; i<d2phi_size; i++)
  {
    second_u += d2phi[i][qp]*soln(dof_indices[i]);
  }
}
