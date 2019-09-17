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
  if (getNumberInputs() != 5 && getNumberInputs() != 6)
    mooseError("In ", _name, ": numbergetNumberInputs() from LAROMData is not 5 or 6");
  if (getNumberOutputs() != 3)
    mooseError("In ", _name, ": number_num_outputs from LAROMData is not 3");
  if (!getDegree() || getDegree() > 4)
    mooseError("In ", _name, ": getDegree() must be 1, 2, 3 or 4.");

  if (getNumberRomCoefficients() != MathUtils::pow(getDegree(), getNumberInputs()))
    mooseError("In ", _name, ": getNumberRomCoefficients() is incorrect");
  if (getDegree() > 4)
    mooseError("In ", _name, ": Maximum allowed Legendre degree is 3 + constant.");

  // Check that transform makes sense
  for (unsigned int i = 0; i < getNumberOutputs(); ++i)
    for (unsigned int j = 0; j < getNumberInputs(); ++j)
      if (transform[i][j] != 0 && transform[i][j] != 1 && transform[i][j] != 2)
        mooseError("In ", _name, ": transform has an invalid function type");

  // Check sizes
  if (transform.size() != getNumberOutputs() || transform[0].size() != getNumberInputs())
    mooseError("In ", _name, ": transform is the wrong shape!");
  if (transform_coef.size() != getNumberOutputs() || transform_coef[0].size() != getNumberInputs())
    mooseError("In ", _name, ": transform_coef is the wrong shape!");
  if (ceofs.size() != getNumberOutputs() || ceofs[0].size() != getNumberRomCoefficients())
    mooseError("In ", _name, ": coefs is the wrong shape!");
  if (input_limits.size() != getNumberInputs() || input_limits[0].size() != 2)
    mooseError("In ", _name, ": input_limits is the wrong shape!");

  Moose::out << "ROM model info:\n  name:\t" << _name << "\n  number of outputs:\t"
             << getNumberOutputs() << "\n  number of inputs:\t" << getNumberInputs()
             << "\n  degree (max Legendre degree + constant):\t" << getDegree()
             << "\n  number of coefficients:\t" << getNumberRomCoefficients() << std::endl;
}

unsigned int
LAROMData::getNumberInputs() const
{
  auto v = getTransform();
  if (!v.size())
    mooseError("In ", _name, ": getTransform is the wrong size");
  return v[0].size();
}

unsigned int
LAROMData::getNumberOutputs() const
{
  auto v = getTransform();
  return v.size();
}

unsigned int
LAROMData::getNumberRomCoefficients() const
{
  auto v = getCoefs();
  if (!v.size())
    mooseError("In ", _name, ": getCoefs is the wrong size");
  return v[0].size();
}

bool
LAROMData::checkForEnvironmentFactor() const
{
  if (getNumberInputs() == 6)
    return true;
  return false;
}

std::vector<std::vector<std::vector<Real>>>
LAROMData::getTransformedLimits() const
{
  std::vector<std::vector<std::vector<Real>>> transformed_limits(
      getNumberOutputs(), std::vector<std::vector<Real>>(getNumberInputs(), std::vector<Real>(2)));

  auto transform = getTransform();
  auto input_limits = getInputLimits();
  auto transform_coefs = getTransformCoefs();

  for (unsigned int i = 0; i < getNumberOutputs(); ++i)
  {
    for (unsigned int j = 0; j < getNumberInputs(); ++j)
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
  std::vector<std::vector<unsigned int>> v(getNumberRomCoefficients(),
                                           std::vector<unsigned int>(getNumberInputs()));
  auto degree = getDegree();

  for (unsigned int numcoeffs = 0; numcoeffs < getNumberRomCoefficients(); ++numcoeffs)
    for (unsigned int invar = 0; invar < getNumberInputs(); ++invar)
      v[numcoeffs][invar] = numcoeffs / MathUtils::pow(degree, invar) % degree;

  return v;
}
