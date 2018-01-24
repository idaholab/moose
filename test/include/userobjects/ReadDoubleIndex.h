/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef READDOUBLEINDEX_H
#define READDOUBLEINDEX_H

#include "GeneralUserObject.h"

class ReadDoubleIndex;

template <>
InputParameters validParams<ReadDoubleIndex>();

/**
 * User Object for testing double index parsing
 */
class ReadDoubleIndex : public GeneralUserObject
{
public:
  ReadDoubleIndex(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  const std::vector<std::vector<Real>> & _real_di;
  const std::vector<std::vector<unsigned int>> & _uint_di;
  const std::vector<std::vector<int>> & _int_di;
  const std::vector<std::vector<long>> & _long_di;
  const std::vector<std::vector<SubdomainID>> & _subid_di;
  const std::vector<std::vector<BoundaryID>> & _bid_di;

  const std::vector<std::vector<std::string>> & _str_di;
  const std::vector<std::vector<FileName>> & _file_di;
  const std::vector<std::vector<FileNameNoExtension>> & _file_no_di;

  const std::vector<std::vector<MeshFileName>> & _mesh_file_di;
  const std::vector<std::vector<SubdomainName>> & _subdomain_name_di;
  const std::vector<std::vector<BoundaryName>> & _boundary_name_di;
  const std::vector<std::vector<FunctionName>> & _function_name_di;
  const std::vector<std::vector<UserObjectName>> & _userobject_name_di;
  const std::vector<std::vector<IndicatorName>> & _indicator_name_di;

  const std::vector<std::vector<MarkerName>> & _marker_name_di;
  const std::vector<std::vector<MultiAppName>> & _multiapp_name_di;
  const std::vector<std::vector<PostprocessorName>> & _postprocessor_name_di;
  const std::vector<std::vector<VectorPostprocessorName>> & _vector_postprocessor_name_di;
  const std::vector<std::vector<OutputName>> & _output_name_di;
  const std::vector<std::vector<MaterialPropertyName>> & _material_property_name_di;
};

#endif /* ReadDoubleIndex_H */
