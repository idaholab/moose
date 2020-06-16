//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunction.h"
#include "GaussianProcessTrainer.h"
#include "MooseError.h"
#include "DataIO.h"
#include "libmesh/auto_ptr.h"

// For computing legendre quadrature
#include "libmesh/dense_matrix_impl.h"

#include <cmath>

namespace CovarianceFunction
{

std::unique_ptr<CovarianceKernel>
makeCovarianceKernel(const int type_id, const GaussianProcessTrainer * gp)
{
  //DenseMatrix<Real> test_theta(2,3);
  std::vector<Real> test_vec=gp->getParam<std::vector<Real>>("length_factor");
  // std::cout << "type_id" << type_id <<'\n';
  // std::cout << "lf_test" << test_vec.at(1) <<'\n';
  if (type_id==1)
    return libmesh_make_unique<SquaredExponential>(test_vec);

  ::mooseError("Unknown covariance function type");
  return nullptr;
}

void
CovarianceKernel::store(std::ostream & /*stream*/, void * /*context*/) const
{
  // Cannot be pure virtual because for dataLoad operations the base class must be constructed
  ::mooseError("Covariance Function child class must override 'store()' method.");
}

DenseMatrix<Real>
CovarianceKernel::compute_matrix(const DenseMatrix<Real> /*x*/, const DenseMatrix<Real> /*xp*/) const
{
  ::mooseError("Covariance function has not been implemented.");
  DenseMatrix<Real> tmp(1,1);
  return tmp;
}

void CovarianceKernel::set_signal_variance(Real sig_f)
{
  _sigma_f = sig_f;
}

SquaredExponential::SquaredExponential(const std::vector<Real> lf)
  : CovarianceKernel(), _length_factor(lf)
{
}

DenseMatrix<Real> SquaredExponential::compute_matrix(const DenseMatrix<Real> x, const DenseMatrix<Real> xp) const
{
  unsigned int num_samples_x = x.m();
  unsigned int num_samples_xp = xp.m();
  unsigned int num_params_x = x.n();
  unsigned int num_params_xp = xp.n();

  if (num_params_x!=num_params_xp){
    ::mooseError("Number of parameters do not match in covariance kernel calculation");}

  DenseMatrix<Real> K(num_samples_x,num_samples_xp);

  for (unsigned int ii=0; ii<num_samples_x; ii++){
    for (unsigned int jj=0; jj<num_samples_xp; jj++){
        //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
        Real val =0;
        for (unsigned int kk=0; kk<num_params_x; kk++){
          //Compute distance per parameter
          //std::cout << kk << "   " << _length_factor.at(kk) << '\n';
          val += std::pow(( x(ii,kk)-  xp(jj,kk) ),2) / (std::pow(_length_factor.at(kk),2));
        }
        //std::cout << _sigma_f << '\n';
        val = _sigma_f* std::exp(-val/2.0);
        K(ii,jj) = val;
      }
  }

  return K;
}

void
SquaredExponential::store(std::ostream & stream, void * context) const
{
  std::string type = "Squared Exponential";
  dataStore(stream, type, context);
  dataStore(stream, _sigma_f, context);
  dataStore(stream, _sigma_n, context);
  //dataStore(stream, _length_factor, context);
  unsigned int n = _length_factor.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ii++){
    dataStore(stream, _length_factor.at(ii), context);}
}

}// namespace

template <>
void
dataStore(std::ostream & stream,
          std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
          void * context)
{
  ptr->store(stream, context);
}

template <>
void
dataLoad(std::istream & stream,
         std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
         void * context)
{
   std::string covar_type;
   dataLoad(stream, covar_type, context);
   if (covar_type == "Squared Exponential")
   {
     Real sigma_f, sigma_n;
     dataLoad(stream, sigma_f, context);
     dataLoad(stream, sigma_n, context);
     unsigned int n;
     dataLoad(stream, n, context);
     std::vector<Real> length_factor(n);
     for (unsigned int ii = 0; ii < n; ii++){
       dataLoad(stream, length_factor.at(ii), context);}
     ptr = libmesh_make_unique<CovarianceFunction::SquaredExponential>(length_factor);
     ptr->set_signal_variance(sigma_f);
   }
   else
     ::mooseError("Unknown Covariance Function: ", covar_type);
}
