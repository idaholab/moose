//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DislocationDensityFileReader.h"

#include <fstream>

registerMooseObject("PhaseFieldApp", DislocationDensityFileReader);

InputParameters
DislocationDensityFileReader::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Read dislocation density data from a file and provide it to other objects.");
  params.addRequiredParam<FileName>("file_name", "dislocation density data file name");
  params.addParam<unsigned int>("lines_to_skip", 4, "number of header lines to skip in the file");

  return params;
}

DislocationDensityFileReader::DislocationDensityFileReader(const InputParameters & params)
  : GeneralUserObject(params),
    _file_name(getParam<FileName>("file_name")),
    _lines_to_skip(getParam<unsigned int>("lines_to_skip"))
{
  readFile();
}

unsigned int
DislocationDensityFileReader::getGrainNum() const
{
  return _densities.size();
}

const Real &
DislocationDensityFileReader::getDensity(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting a dislocation density for an invalid grain ID");
  return _densities[i];
}

void
DislocationDensityFileReader::readFile()
{
  // Read in dislocation densities
  std::ifstream inFile(_file_name.c_str());
  if (!inFile)
    mooseError("Can't open ", _file_name);

  // Skip first n lines
  for (unsigned int i = 0; i < _lines_to_skip; ++i)
    inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Loop over grains
  Real d;
  while (inFile >> d)
    _densities.push_back(Real(d));
}
