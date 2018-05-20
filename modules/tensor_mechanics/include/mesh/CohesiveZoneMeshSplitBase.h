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

#ifndef COHESIVEZONEMESHSPLITBASE_H
#define COHESIVEZONEMESHSPLITBASE_H

#include "MooseMesh.h"

// forward declaration
class CohesiveZoneMeshSplitBase;

template <>
InputParameters validParams<CohesiveZoneMeshSplitBase>();

class CohesiveZoneMeshSplitBase : public MooseMesh
{
public:
  CohesiveZoneMeshSplitBase(const InputParameters & parameters);
  CohesiveZoneMeshSplitBase(const CohesiveZoneMeshSplitBase & other_mesh);

  // empty dtor required for unique_ptr with forward  declarations
  virtual ~CohesiveZoneMeshSplitBase();

  // method to override to implement other mesh splitting algorithms
  virtual void init() override;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  void read(const std::string & file_name);
  virtual ExodusII_IO * exReader() const override { return _exreader.get(); }

  // Get/Set Filename (for meshes read from a file)
  void setFileName(const std::string & file_name) { _file_name = file_name; }
  virtual std::string getFileName() const override { return _file_name; }

protected:
  /// the file_name from whence this mesh came
  std::string _file_name;
  /// the name of the new interface
  std::string _interface_name;
  /// the ID of the new interface
  BoundaryID _interface_id;
  /// the flag to split the interface by block
  bool _split_interface;

  /// Auxiliary object for restart
  std::unique_ptr<ExodusII_IO> _exreader;

  /// check that if split_interface==true interface_id and interface_name are
  /// not set by the user. It also check that the provided interface_id is not
  /// already used
  void checkInputParameter();

  /// given the master and slave blocks this method return the appropriate
  /// boundary id and name
  void findBoundaryNameAndInd(const subdomain_id_type & /*masterBlockID*/,
                              const subdomain_id_type & /*slaveBlockID*/,
                              std::string & /*boundaryName*/,
                              BoundaryID & /*boundaryID*/,
                              BoundaryInfo & /*boundary_info*/);

  std::set<std::pair<std::string, BoundaryID>> _czm_bName_bID_set;

private:
  /// this method finds the first free boundary id
  BoundaryID findFreeBoundaryId();

  /// this method generate the boundary name as
  /// "czm_bM_" + masterBlockID + "=>_bS_" + slaveBlockID;
  std::string generateBoundaryName(const subdomain_id_type & /*masterBlockID*/,
                                   const subdomain_id_type & /*slaveBlockID*/);

  /// this method save the boundary name/id pair
  void mapBoundaryIdAndBoundaryName(BoundaryID & /*boundaryID*/, std::string & /*boundaryName*/);
};

#endif // COHESIVEZONEMESHSPLITBASE_H
