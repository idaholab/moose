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

RestartableModelInterface::RestartableModelInterface(const MooseObject & object,
                                                     const bool read_only,
                                                     const std::string & meta_data_name)
  : _model_object(object),
    _model_meta_data_name(meta_data_name),
    _model_restartable(_model_object.getMooseApp(), "", "", 0, read_only, meta_data_name)
{
  _model_object.getMooseApp().registerRestartableDataMapName(_model_meta_data_name,
                                                             _model_object.name());
}

const FileName &
RestartableModelInterface::getModelDataFileName() const
{
  return _model_object.getParam<FileName>("filename");
}

bool
RestartableModelInterface::hasModelData() const
{
  return _model_object.isParamValid("filename");
}
