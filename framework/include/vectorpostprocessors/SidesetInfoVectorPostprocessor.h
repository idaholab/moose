//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideVectorPostprocessor.h"

class SidesetInfoVectorPostprocessor : public SideVectorPostprocessor
{
public:
  static InputParameters validParams();

  SidesetInfoVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual void threadJoin(const UserObject & y) override;

  struct BoundaryData
  {
    BoundaryData()
      : area(0),
        centroid(Point()),
        min(Point(std::numeric_limits<Real>::max(),
                  std::numeric_limits<Real>::max(),
                  std::numeric_limits<Real>::max())),
        max(Point(std::numeric_limits<Real>::lowest(),
                  std::numeric_limits<Real>::lowest(),
                  std::numeric_limits<Real>::lowest()))
    {
    }
    Real area;
    Point centroid;
    Point min;
    Point max;
  };

protected:
  /// a helper function for retrieving data from _boundary_info
  Real dataHelper(BoundaryID bid, std::string mdat_tpe) const;

  /// the type of meta data that is written to file
  MultiMooseEnum _meta_data_types;

  /// the type of meta data that is written to file
  std::vector<std::string> _vpp_entry_names;

  /// the sideset id
  VectorPostprocessorValue & _sideset_ids;

  /// the vpp data is stored here
  std::vector<VectorPostprocessorValue *> _meta_data;

  /// all data available through the meta_data_types is always accumulated
  std::map<BoundaryID, BoundaryData> _boundary_data;
};
