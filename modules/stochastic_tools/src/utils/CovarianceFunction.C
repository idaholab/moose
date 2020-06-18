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

MooseEnum
makeCovarianceKernelEnum()
{
  return MooseEnum("squared_exponential=0 exponential=1");
}

std::unique_ptr<CovarianceKernel>
makeCovarianceKernel(const MooseEnum & kernel_type, const GaussianProcessTrainer * gp)
{
  // std::cout << "type_id" << type_id <<'\n';
  // std::cout << "lf_test" << test_vec.at(1) <<'\n';
  if (kernel_type==0){
    //std::cout << "type_id = squared exponential" <<'\n';
    std::unique_ptr<CovarianceKernel> ptr = libmesh_make_unique<SquaredExponential>();
    std::vector<Real> length_factor=gp->getParam<std::vector<Real>>("length_factor");
    ptr->set_length_factor(length_factor);
    return ptr;
  }

  if (kernel_type==1){
    //std::cout << "type_id = squared exponential" <<'\n';
    std::unique_ptr<CovarianceKernel> ptr = libmesh_make_unique<Exponential>();
    std::vector<Real> length_factor=gp->getParam<std::vector<Real>>("length_factor");
    Real gamma=gp->getParam<Real>("gamma");
    ptr->set_length_factor(length_factor);
    ptr->set_gamma(gamma);
    return ptr;
  }

  ::mooseError("Unknown covariance function type");
  return nullptr;
}

void
CovarianceKernel::store(std::ostream & /*stream*/, void * /*context*/) const
{
  // Cannot be pure virtual because for dataLoad operations the base class must be constructed
  ::mooseError("Covariance Function child class must override 'store()' method.");
}

RealEigenMatrix
CovarianceKernel::compute_matrix(const RealEigenMatrix /*x*/, const RealEigenMatrix /*xp*/) const
{
  ::mooseError("Covariance function has not been implemented.");
  RealEigenMatrix tmp(1,1);
  return tmp;
}

void CovarianceKernel::set_signal_variance(const Real sig_f)
{
  _sigma_f = sig_f;
}

void CovarianceKernel::set_length_factor(const std::vector<Real> length_factor)
{
  _length_factor = length_factor;
}

void CovarianceKernel::set_gamma(const Real /*gamma*/)
{
  ::mooseError("Attempting to set gamma hyper parameter for covariance function which does not utilize.");
}

//begin squared_exponential
SquaredExponential::SquaredExponential()
  : CovarianceKernel()
{
}

RealEigenMatrix SquaredExponential::compute_matrix(const RealEigenMatrix x, const RealEigenMatrix xp) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();
  unsigned int num_params_xp = xp.cols();

  if (num_params_x!=num_params_xp){
    ::mooseError("Number of parameters do not match in covariance kernel calculation");}

  RealEigenMatrix K(num_samples_x,num_samples_xp);

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
//end squared_exponential


//begin exponential
Exponential::Exponential()
  : CovarianceKernel()
{
}

RealEigenMatrix Exponential::compute_matrix(const RealEigenMatrix x, const RealEigenMatrix xp) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();
  unsigned int num_params_xp = xp.cols();

  if (num_params_x!=num_params_xp){
    ::mooseError("Number of parameters do not match in covariance kernel calculation");}

  RealEigenMatrix K(num_samples_x,num_samples_xp);

  for (unsigned int ii=0; ii<num_samples_x; ii++){
    for (unsigned int jj=0; jj<num_samples_xp; jj++){
        //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
        Real r_scaled =0;
        //Compute distance per parameter, scaled by length factor
        for (unsigned int kk=0; kk<num_params_x; kk++){
          r_scaled += pow( (x(ii,kk)- xp(jj,kk))/_length_factor.at(kk), 2);
        }
        r_scaled = sqrt(r_scaled);
        //std::cout << _sigma_f << '\n';
        K(ii,jj) =  _sigma_f* std::exp(-pow(r_scaled,_gamma));
      }
  }

  return K;
}

void
Exponential::store(std::ostream & stream, void * context) const
{
  std::string type = "Exponential";
  dataStore(stream, type, context);
  dataStore(stream, _sigma_f, context);
  dataStore(stream, _sigma_n, context);
  //dataStore(stream, _length_factor, context);
  unsigned int n = _length_factor.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ii++){
    dataStore(stream, _length_factor.at(ii), context);
  }
  dataStore(stream, _gamma, context);
}

void Exponential::set_gamma(const Real gamma)
{
  _gamma = gamma;
}
//end exponential

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
     ptr = libmesh_make_unique<CovarianceFunction::SquaredExponential>();
     ptr->set_length_factor(length_factor);
     ptr->set_signal_variance(sigma_f);
   }
   else if (covar_type == "Exponential")
   {
     Real sigma_f, sigma_n;
     dataLoad(stream, sigma_f, context);
     dataLoad(stream, sigma_n, context);
     unsigned int n;
     dataLoad(stream, n, context);
     std::vector<Real> length_factor(n);
     for (unsigned int ii = 0; ii < n; ii++){
       dataLoad(stream, length_factor.at(ii), context);}
     ptr = libmesh_make_unique<CovarianceFunction::Exponential>();
     ptr->set_length_factor(length_factor);
     ptr->set_signal_variance(sigma_f);
     Real gamma;
     dataLoad(stream, gamma, context);
     ptr->set_gamma(gamma);
   }
   else
     ::mooseError("Unknown Covariance Function: ", covar_type);
}
