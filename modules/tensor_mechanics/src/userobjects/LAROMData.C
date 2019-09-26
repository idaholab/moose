//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  saturation as a function of effective saturation, and its derivs wrt effective saturation
//
#include "LAROMData.h"

#include "MathUtils.h"

template <>
InputParameters
validParams<LAROMData>()
{
  return validParams<GeneralUserObject>();
}

LAROMData::LAROMData(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
LAROMData::initialSetup()
{
  auto transform = getTransform();
  auto transform_coef = getTransformCoefs();
  auto input_limits = getInputLimits();
  auto ceofs = getCoefs();

  // check sizes make sense
  if (getNumberOfInputs() != 5 && getNumberOfInputs() != 6)
    mooseError("In ", _name, ": numbergetNumberOfInputs() from LAROMData is not 5 or 6");
  if (getNumberOfOutputs() != 3)
    mooseError("In ", _name, ": number_num_outputs from LAROMData is not 3");
  if (!getDegree() || getDegree() > 4)
    mooseError("In ", _name, ": getDegree() must be 1, 2, 3 or 4.");

  if (getNumberOfRomCoefficients() != MathUtils::pow(getDegree(), getNumberOfInputs()))
    mooseError("In ", _name, ": getNumberOfRomCoefficients() is incorrect");
  if (getDegree() > 4)
    mooseError("In ", _name, ": Maximum allowed Legendre degree is 3 + constant.");

  // Check that transform makes sense
  for (unsigned int i = 0; i < getNumberOfOutputs(); ++i)
    for (unsigned int j = 0; j < getNumberOfInputs(); ++j)
      if (transform[i][j] != 0 && transform[i][j] != 1 && transform[i][j] != 2)
        mooseError("In ", _name, ": transform has an invalid function type");

  // Check sizes
  if (transform.size() != getNumberOfOutputs() || transform[0].size() != getNumberOfInputs())
    mooseError("In ", _name, ": transform is the wrong shape!");
  if (transform_coef.size() != getNumberOfOutputs() || transform_coef[0].size() != getNumberOfInputs())
    mooseError("In ", _name, ": transform_coef is the wrong shape!");
  if (ceofs.size() != getNumberOfOutputs() || ceofs[0].size() != getNumberOfRomCoefficients())
    mooseError("In ", _name, ": coefs is the wrong shape!");
  if (input_limits.size() != getNumberOfInputs() || input_limits[0].size() != 2)
    mooseError("In ", _name, ": input_limits is the wrong shape!");

  Moose::out << "ROM model info:\n  name:\t" << _name << "\n  number of outputs:\t"
             << getNumberOfOutputs() << "\n  number of inputs:\t" << getNumberOfInputs()
             << "\n  degree (max Legendre degree + constant):\t" << getDegree()
             << "\n  number of coefficients:\t" << getNumberOfRomCoefficients() << std::endl;
}

unsigned int
LAROMData::getNumberOfInputs() const
{
  auto v = getTransform();
  if (!v.size())
    mooseError("In ", _name, ": getTransform is the wrong size");
  return v[0].size();
}

unsigned int
LAROMData::getNumberOfOutputs() const
{
  auto v = getTransform();
  return v.size();
}

unsigned int
LAROMData::getNumberOfRomCoefficients() const
{
  auto v = getCoefs();
  if (!v.size())
    mooseError("In ", _name, ": getCoefs is the wrong size");
  return v[0].size();
}

bool
LAROMData::checkForEnvironmentFactor() const
{
  if (getNumberOfInputs() == 6)
    return true;
  return false;
}

std::vector<std::vector<std::vector<Real>>>
LAROMData::getTransformedLimits() const
{
  std::vector<std::vector<std::vector<Real>>> transformed_limits(
      getNumberOfOutputs(), std::vector<std::vector<Real>>(getNumberOfInputs(), std::vector<Real>(2)));

  auto transform = getTransform();
  auto input_limits = getInputLimits();
  auto transform_coefs = getTransformCoefs();

  for (unsigned int i = 0; i < getNumberOfOutputs(); ++i)
  {
    for (unsigned int j = 0; j < getNumberOfInputs(); ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        if (transform[i][j] == 2)
          transformed_limits[i][j][k] = std::exp(input_limits[j][k] / transform_coefs[i][j]);
        else if (transform[i][j] == 1)
          transformed_limits[i][j][k] = std::log(input_limits[j][k] + transform_coefs[i][j]);
        else
          transformed_limits[i][j][k] = input_limits[j][k];
      }
    }
  }

  return transformed_limits;
}

std::vector<std::vector<unsigned int>>
LAROMData::getMakeFrameHelper() const
{
  std::vector<std::vector<unsigned int>> v(getNumberOfRomCoefficients(),
                                           std::vector<unsigned int>(getNumberOfInputs()));
  auto degree = getDegree();

  for (unsigned int numcoeffs = 0; numcoeffs < getNumberOfRomCoefficients(); ++numcoeffs)
    for (unsigned int invar = 0; invar < getNumberOfInputs(); ++invar)
      v[numcoeffs][invar] = numcoeffs / MathUtils::pow(degree, invar) % degree;

  return v;
}
