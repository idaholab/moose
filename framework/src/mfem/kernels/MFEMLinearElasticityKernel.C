//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMLinearElasticityKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMLinearElasticityKernel);

InputParameters
MFEMLinearElasticityKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The isotropic linear elasticity operator with weak form "
      "$(c_{ikjl} \\nabla u_j, \\nabla v_i)$, to be added to an MFEM problem, where "
      "$c_{ikjl}$ is the isotropic elasticity tensor, "
      "$c_{ikjl} = \\lambda \\delta_{ik} \\delta_{jl} + \\mu \\left( \\delta_{ij} \\delta_{kl} + "
      "\\delta_{il} \\delta_{jk} \\right)$, "
      "$\\lambda$ is the first Lame parameter, $\\lambda = \\frac{E\\nu}{(1-2\\nu)(1+\\nu)}$, "
      "$\\mu$ is the second Lame parameter, $\\mu = \\frac{E}{2(1+\\nu)}$, "
      "where $E$ is Young's modulus and $\\nu$ is Poisson's ratio.");

  params.addParam<MFEMScalarCoefficientName>(
      "lambda", "1.", "Name of MFEM Lame constant lambda to multiply the div(u)*I term by");
  params.addParam<MFEMScalarCoefficientName>(
      "mu", "1.", "Name of MFEM Lame constant mu to multiply the gradients term by");

  return params;
}

MFEMLinearElasticityKernel::MFEMLinearElasticityKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _lambda_name(getParam<MFEMScalarCoefficientName>("lambda")),
    _mu_name(getParam<MFEMScalarCoefficientName>("mu")),
    _lambda(getScalarCoefficient(_lambda_name)),
    _mu(getScalarCoefficient(_mu_name))
{
}

mfem::BilinearFormIntegrator *
MFEMLinearElasticityKernel::createBFIntegrator()
{
  return new mfem::ElasticityIntegrator(_lambda, _mu);
}

#endif
