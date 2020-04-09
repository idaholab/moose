//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EulerAngleFileReader.h"

#include <fstream>

registerMooseObject("TensorMechanicsApp", EulerAngleFileReader);

InputParameters
EulerAngleFileReader::validParams()
{
  InputParameters params = EulerAngleProvider::validParams();
  params.addClassDescription("Read Euler angle data from a file and provide it to other objects.");
  params.addRequiredParam<FileName>("file_name", "Euler angle data file name");
  return params;
}

EulerAngleFileReader::EulerAngleFileReader(const InputParameters & params)
  : EulerAngleProvider(params), _file_name(getParam<FileName>("file_name"))
{
  readFile();
}

unsigned int
EulerAngleFileReader::getGrainNum() const
{
  return _angles.size();
}

const EulerAngles &
EulerAngleFileReader::getEulerAngles(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
  return _angles[i];
}

void
EulerAngleFileReader::readFile()
{
  // Read in Euler angles from _file_name
  std::ifstream inFile(_file_name.c_str());
  if (!inFile)
    mooseError("Can't open ", _file_name);

  // Skip first 4 lines
  for (unsigned int i = 0; i < 4; ++i)
    inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // The angle files contain a fourth column with weights that we ignore in this version
  Real weight;

  // Loop over grains
  EulerAngles a;
  while (inFile >> a.phi1 >> a.Phi >> a.phi2 >> weight)
    _angles.push_back(EulerAngles(a));
}
