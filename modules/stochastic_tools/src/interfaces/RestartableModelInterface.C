//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableModelInterface.h"

InputParameters
RestartableModelInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<FileName>(
      "filename", "The name of the file which will be associated with the saved/loaded data.");
  return params;
}

RestartableModelInterface::RestartableModelInterface(const MooseObject * object,
                                                     const bool read_only,
                                                     const std::string & meta_data_name)
  : _model_meta_data_name(meta_data_name),
    _object(object),
    _restartable(_object->getMooseApp(), "", "", 0, false, meta_data_name)
{
  _object->getMooseApp().registerRestartableDataMapName(_model_meta_data_name, _object->name());
}
