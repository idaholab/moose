//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPerfectlyMatchedLayerFunction.h"
#include "MFEMPMLMatrixCoefficient.h"
#include "MFEMPMLScalarCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMPerfectlyMatchedLayerFunction);

InputParameters
MFEMPerfectlyMatchedLayerFunction::validParams()
{
  InputParameters params = Function::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription("Declares an MFEM coefficient applying the complex coordinate stretch "
                             "of a perfectly matched layer to the base coefficient of a bilinear "
                             "form.");
  params.addRequiredParam<MooseEnum>("coefficient_type",
                                     MooseEnum("scalar=0 matrix=1"),
                                     "Whether to declare a scalar or a matrix coefficient. Only "
                                     "the two dimensional curl curl term takes a scalar.");
  params.addRequiredParam<MooseEnum>("tensor",
                                     MooseEnum("curl=0 field=1"),
                                     "Whether the coefficient stretches the curl of the field or "
                                     "the field itself. Which applies is set by the quantity the "
                                     "bilinear form integrates.");
  params.addRequiredParam<MooseEnum>("component",
                                     MooseEnum("real=0 imaginary=1"),
                                     "Part of the complex coefficient to declare. The complex "
                                     "system is assembled from two real bilinear forms.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient",
      "1.",
      "Name of the base scalar coefficient the stretch scales, such as the reluctivity or the mass "
      "coefficient of the bilinear form this is passed to.");
  params.addRequiredRangeCheckedParam<Real>(
      "decay_coefficient",
      "decay_coefficient > 0",
      "Decay coefficient of the layer, equal to the tuning constant divided by the wavenumber.");
  params.addRangeCheckedParam<Real>(
      "decay_polynomial",
      2.0,
      "decay_polynomial > 1",
      "Polynomial order of the stretch profile. It must exceed one so that the displacement and "
      "its derivative both vanish where the layer meets the rest of the domain, leaving nothing "
      "for a wave entering the layer to reflect off.");
  return params;
}

MFEMPerfectlyMatchedLayerFunction::MFEMPerfectlyMatchedLayerFunction(
    const InputParameters & parameters)
  : Function(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _stretch_vec(getStretch()),
    _base_coefficient(nullptr)
{
  const bool imaginary = getParam<MooseEnum>("component") == "imaginary";
  const bool field = getParam<MooseEnum>("tensor") == "field";
  auto & coefficients = getMFEMProblem().getCoefficients();

  if (getParam<MooseEnum>("coefficient_type") == "scalar")
  {
    if (field)
      mooseError("Only the tensor stretching the curl of the field reduces to a scalar. Declare a "
                 "matrix coefficient for 'tensor = field'.");
    if (_stretch_vec.dim() != 2)
      mooseError("The tensor stretching the curl of the field only reduces to a scalar in two "
                 "dimensions, where the curl of a vector field is a scalar. Declare a matrix "
                 "coefficient on a three dimensional mesh.");

    coefficients.declareScalar<MFEMPMLScalarCoefficient>(name(),
                                                         _stretch_vec,
                                                         _base_coefficient,
                                                         imaginary
                                                             ? MFEMPMLScalarCoefficient::IMAGINARY
                                                             : MFEMPMLScalarCoefficient::REAL);
  }
  else
  {
    if (!field && _stretch_vec.dim() != 3)
      mooseError("In two dimensions the curl of a vector field is a scalar, so the tensor "
                 "stretching it is not a matrix. Declare a scalar coefficient instead.");

    coefficients.declareMatrix<MFEMPMLMatrixCoefficient>(
        name(),
        _stretch_vec,
        _base_coefficient,
        field ? MFEMPMLMatrixCoefficient::FIELD : MFEMPMLMatrixCoefficient::CURL,
        imaginary ? MFEMPMLMatrixCoefficient::IMAGINARY : MFEMPMLMatrixCoefficient::REAL);
  }
}

MFEMProblem &
MFEMPerfectlyMatchedLayerFunction::getMFEMProblem() const
{
  auto * const problem = dynamic_cast<MFEMProblem *>(&_mci_feproblem);
  if (!problem)
    mooseError("A perfectly matched layer function declares an MFEM coefficient, so it can only be "
               "used with an MFEMProblem.");

  return *problem;
}

void
MFEMPerfectlyMatchedLayerFunction::initialSetup()
{
  _base_coefficient = &getMFEMProblem().getCoefficients().getScalarCoefficient(
      getParam<MFEMScalarCoefficientName>("coefficient"));
}

std::string
MFEMPerfectlyMatchedLayerFunction::stretchName() const
{
  std::string name = "_pml_stretch_vec";
  for (const auto attribute : _subdomain_attributes)
    name += "_" + std::to_string(attribute);

  return name + "_" + std::to_string(getParam<Real>("decay_coefficient")) + "_" +
         std::to_string(getParam<Real>("decay_polynomial"));
}

MFEMPMLStretchVector &
MFEMPerfectlyMatchedLayerFunction::getStretch()
{
  if (getSubdomainAttributes().IsEmpty())
    mooseError("A perfectly matched layer is the block it is restricted to, so 'block' must name "
               "the subdomains forming the layer.");

  auto & coefficients = getMFEMProblem().getCoefficients();
  const std::string name = stretchName();

  // A coefficient declared without blocks counts as defined on every one of them, so this asks
  // whether another function acting on the same layer has already built the stretch.
  if (coefficients.vectorPropertyIsDefined(name, std::to_string(getSubdomainAttributes()[0])))
    return dynamic_cast<MFEMPMLStretchVector &>(coefficients.getVectorCoefficient(name));

  return coefficients.declareVector<MFEMPMLStretchVector>(
      name,
      getMFEMProblem().mesh().getMFEMParMesh(),
      getSubdomainAttributes(),
      getParam<Real>("decay_coefficient"),
      getParam<Real>("decay_polynomial"),
      getMFEMProblem().getComm());
}

#endif
