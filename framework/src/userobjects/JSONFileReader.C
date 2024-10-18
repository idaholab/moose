//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JSONFileReader.h"

#include "MooseUtils.h"
#include "json.h"

registerMooseObject("MooseApp", JSONFileReader);

InputParameters
JSONFileReader::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  // Add parameters
  params.addRequiredParam<FileName>("filename", "The path to the file including its name");
  // we run this object once at the initialization by default
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.addClassDescription("Loads a JSON file and makes it content available to consumers");
  return params;
}

JSONFileReader::JSONFileReader(const InputParameters & parameters)
  : GeneralUserObject(parameters), _filename(getParam<FileName>("filename"))
{
  read(_filename);
}

void
JSONFileReader::read(const FileName & filename)
{
  MooseUtils::checkFileReadable(filename);
  // Read the JSON database
  std::ifstream jsondata(filename);
  jsondata >> _root;
}
