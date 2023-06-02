//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralUserObject.h"

// Forward declarations
namespace libMesh
{
class ExodusII_IO;
class EquationSystems;
class System;
class MeshFunction;
template <class T>
class NumericVector;
}

/**
 * User object that reads an existing solution from an input file and
 * uses it in the current simulation.
 */
class SolutionUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  SolutionUserObject(const InputParameters & parameters);
  virtual ~SolutionUserObject(); // empty dtor required for unique_ptr with forward declarations

  /**
   * When reading ExodusII files, this will update the interpolation times
   */
  virtual void timestepSetup() override;

  /**
   * Returns the local index for a given variable name
   * @param var_name The name of the variable for which the index is located
   * @return The local index of the variable
   */
  unsigned int getLocalVarIndex(const std::string & var_name) const;

  /**
   * Returns a value at a specific location and variable checking for multiple values and weighting
   * these values to
   * obtain a single unique value (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  Real pointValueWrapper(Real t,
                         const Point & p,
                         const std::string & var_name,
                         const MooseEnum & weighting_type = weightingType(),
                         const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns a value at a specific location and variable (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param local_var_index The local index of the variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  Real pointValue(Real t,
                  const Point & p,
                  const unsigned int local_var_index,
                  const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns a value at a specific location and variable (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  Real pointValue(Real t,
                  const Point & p,
                  const std::string & var_name,
                  const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns a value at a specific location and variable for cases where the solution is
   * multivalued at element faces
   * Use pointValue for continuous shape functions or if you are sure your point is within an
   * element!
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param local_var_index The local index of the variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  std::map<const Elem *, Real>
  discontinuousPointValue(Real t,
                          Point pt,
                          const unsigned int local_var_index,
                          const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns a value at a specific location and variable for cases where the solution is
   * multivalued at element faces
   * Use pointValue for continuous shape functions or if you are sure your point is within an
   * element!
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  std::map<const Elem *, Real>
  discontinuousPointValue(Real t,
                          const Point & p,
                          const std::string & var_name,
                          const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns the gradient at a specific location and variable checking for multiple values and
   * weighting these values to
   * obtain a single unique value (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  RealGradient
  pointValueGradientWrapper(Real t,
                            const Point & p,
                            const std::string & var_name,
                            const MooseEnum & weighting_type = weightingType(),
                            const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns the gradient at a specific location and variable (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  RealGradient
  pointValueGradient(Real t,
                     const Point & p,
                     const std::string & var_name,
                     const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns the gradient at a specific location and variable (see SolutionFunction)
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param local_var_index The local index of the variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  RealGradient
  pointValueGradient(Real t,
                     Point pt,
                     const unsigned int local_var_index,
                     const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns the gradient at a specific location and variable for cases where the gradient is
   * multivalued (e.g. at element faces)
   * Use pointValueGradient for continuous gradients or if you are sure your point is within an
   * element!
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param var_name The variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  std::map<const Elem *, RealGradient> discontinuousPointValueGradient(
      Real t,
      const Point & p,
      const std::string & var_name,
      const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Returns the gradient at a specific location and variable for cases where the gradient is
   * multivalued (e.g. at element faces)
   * Use pointValueGradient for continuous gradients or if you are sure your point is within an
   * element!
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param p The location at which to return a value
   * @param local_var_index The local index of the variable to be evaluated
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @return The desired value for the given variable at a location
   */
  std::map<const Elem *, RealGradient> discontinuousPointValueGradient(
      Real t,
      Point pt,
      const unsigned int local_var_index,
      const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Return a value directly from a Node
   * @param node A pointer to the node at which a value is desired
   * @param var_name The variable from which to extract a value
   * @return The desired value for the given node and variable name
   */
  Real directValue(const Node * node, const std::string & var_name) const;

  /**
   * Return a value from the centroid of an element
   * @param elem A pointer to the element at which a value is desired
   * @param var_name The variable from which to extract a value
   * @return The desired value for the given element and variable name
   */
  Real directValue(const Elem * elem, const std::string & var_name) const;

  /**
   * Returns a value of a global variable
   * @param t The time at which to extract (not used, it is handled automatically when reading the
   * data)
   * @param var_name The variable from which to extract a value
   * @return The desired value for the given variable
   */
  Real scalarValue(Real t, const std::string & var_name) const;

  // Required pure virtual function (not used)
  virtual void initialize() override;

  // Required pure virtual function (not used)
  virtual void finalize() override;

  // Required pure virtual function (not used)
  virtual void execute() override;

  /// Initialize the System and Mesh objects for the solution being read
  virtual void initialSetup() override;

  const std::vector<std::string> & variableNames() const;

  bool isVariableNodal(const std::string & var_name) const;

  static MooseEnum weightingType()
  {
    return MooseEnum("found_first=1 average=2 smallest_element_id=4 largest_element_id=8",
                     "found_first");
  }

  /**
   * Return the spatial dimension of the mesh file
   * @return The spatial dimension of the mesh file
   */
  unsigned int getMeshFileDimension() const { return _mesh->spatial_dimension(); }

  /**
   * Return the name of the mesh file this object read the solution from
   */
  const std::string getMeshFileName() const { return _mesh_file; }

  /**
   * Get the map from block name to block ID. Only works for ExodusII files.
   *
   * @return Map from block name to block ID
   */
  const std::map<SubdomainName, SubdomainID> & getBlockNamesToIds() const
  {
    return _block_name_to_id;
  }

  /**
   * Get the map from block id to block name. Only works for ExodusII files.
   *
   * @return Map from block ID to block name
   */
  const std::map<SubdomainID, SubdomainName> & getBlockIdsToNames() const
  {
    return _block_id_to_name;
  }

  /**
   * Get the type of file that was read
   * @return Returns a MooseEnum that specifies the type of file read
   */
  MooseEnum getSolutionFileType() const;

protected:
  /**
   * Method for reading XDA mesh and equation systems file(s)
   * This method is called by the constructor when 'file_type = xda' is set
   * in the input file.
   */
  void readXda();

  /**
   * Method for reading an ExodusII file, which is called when
   * 'file_type = exodusII is set in the input file.
   */
  void readExodusII();

  /**
   * Method for extracting value of solution based on the DOF,
   * this is called by the public overloaded function that accept
   * a node or element pointer.
   * @param dof_index The DOF of the solution desired
   * @return The solution at the given DOF
   */
  virtual Real directValue(dof_id_type dof_index) const;

  /**
   * Updates the times for interpolating ExodusII data
   * @param time The new time
   */
  void updateExodusTimeInterpolation(Real time);

  /**
   * Updates the time indices to interpolate between for ExodusII data
   * @param time The new time
   */
  bool updateExodusBracketingTimeIndices(Real time);

  /**
   * A wrapper method for calling the various MeshFunctions used for reading the data
   * @param p The location at which data is desired
   * @param local_var_index The local index of the variable to extract data from
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @param func_num The MeshFunction index to use (1 = _mesh_function; 2 = _mesh_function2)
   */
  Real evalMeshFunction(const Point & p,
                        const unsigned int local_var_index,
                        unsigned int func_num,
                        const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * A wrapper method for calling the various MeshFunctions that calls the mesh function
   * functionality for evaluating discontinuous shape functions near a face (where it's multivalued)
   * @param p The location at which data is desired
   * @param local_var_index The local index of the variable to extract data from
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @param func_num The MeshFunction index to use (1 = _mesh_function; 2 = _mesh_function2)
   */
  std::map<const Elem *, Real>
  evalMultiValuedMeshFunction(const Point & p,
                              const unsigned int local_var_index,
                              unsigned int func_num,
                              const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * A wrapper method interfacing with the libMesh mesh function for evaluating the gradient
   * @param p The location at which data is desired
   * @param local_var_index The local index of the variable to extract data from
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @param func_num The MeshFunction index to use (1 = _mesh_function; 2 = _mesh_function2)
   */
  RealGradient
  evalMeshFunctionGradient(const Point & p,
                           const unsigned int local_var_index,
                           unsigned int func_num,
                           const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * A wrapper method interfacing with the libMesh mesh function that calls the gradient
   * functionality for evaluating potentially discontinuous gradients at element's faces (where it's
   * multivalued)
   * @param p The location at which data is desired
   * @param local_var_index The local index of the variable to extract data from
   * @param subdomain_ids Subdomains IDs where to look for the value, if nullptr look everywhere
   * @param func_num The MeshFunction index to use (1 = _mesh_function; 2 = _mesh_function2)
   */
  std::map<const Elem *, RealGradient> evalMultiValuedMeshFunctionGradient(
      const Point & p,
      const unsigned int local_var_index,
      unsigned int func_num,
      const std::set<subdomain_id_type> * subdomain_ids = nullptr) const;

  /**
   * Read block ID map from the ExodusII file
   */
  void readBlockIdMapFromExodusII();

  /// File type to read (0 = xda; 1 = ExodusII)
  MooseEnum _file_type;

  /// The XDA or ExodusII file that is being read
  std::string _mesh_file;

  /// The XDA file that contians the EquationSystems data (xda only)
  std::string _es_file;

  /// The system name to extract from the XDA file (xda only)
  std::string _system_name;

  /// A list of variables to extract from the read system
  std::vector<std::string> _system_variables;

  /// Stores the local index need by MeshFunction
  std::map<std::string, unsigned int> _local_variable_index;

  /// Stores names of nodal variables
  std::vector<std::string> _nodal_variables;

  /// Stores names of elemental variables
  std::vector<std::string> _elemental_variables;

  /// Stores names of scalar variables
  std::vector<std::string> _scalar_variables;

  /// Current ExodusII time index
  int _exodus_time_index;

  /// Flag for triggering interpolation of ExodusII data
  bool _interpolate_times;

  /// Pointer the libmesh::mesh object
  std::unique_ptr<MeshBase> _mesh;

  /// Pointer to the libmesh::EquationSystems object
  std::unique_ptr<EquationSystems> _es;

  /// Pointer libMesh::System class storing the read solution
  System * _system;

  /// Pointer the libMesh::MeshFunction object that the read data is stored
  std::unique_ptr<MeshFunction> _mesh_function;

  /// Pointer to the libMesh::ExodusII used to read the files
  std::unique_ptr<ExodusII_IO> _exodusII_io;

  /// Pointer to the serial solution vector
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Pointer to second libMesh::EquationSystems object, used for interpolation
  std::unique_ptr<EquationSystems> _es2;

  /// Pointer to a second libMesh::System object, used for interpolation
  System * _system2;

  /// Pointer to second libMesh::MeshFuntion, used for interpolation
  std::unique_ptr<MeshFunction> _mesh_function2;

  /// Pointer to second serial solution, used for interpolation
  std::unique_ptr<NumericVector<Number>> _serialized_solution2;

  /// Interpolation time
  Real _interpolation_time;

  /// Interpolation weight factor
  Real _interpolation_factor;

  /// The times available in the ExodusII file
  const std::vector<Real> * _exodus_times;

  /// Time index 1, used for interpolation
  int _exodus_index1;

  /// Time index 2, used for interpolation
  int _exodus_index2;

  /// Scale parameter
  std::vector<Real> _scale;

  /// scale_multiplier parameter
  std::vector<Real> _scale_multiplier;

  /// Translation
  std::vector<Real> _translation;

  /// vector about which to rotate
  RealVectorValue _rotation0_vector;

  /// angle (in degrees) which to rotate through about vector _rotation0_vector
  Real _rotation0_angle;

  /// Rotation matrix that performs the "_rotation0_angle about rotation0_vector"
  RealTensorValue _r0;

  /// vector about which to rotate
  RealVectorValue _rotation1_vector;

  /// angle (in degrees) which to rotate through about vector _rotation1_vector
  Real _rotation1_angle;

  /// Rotation matrix that performs the "_rotation1_angle about rotation1_vector"
  RealTensorValue _r1;

  /// transformations (rotations, translation, scales) are performed in this order
  MultiMooseEnum _transformation_order;

  /// True if initial_setup has executed
  bool _initialized;

  /// Map from block ID to block names. Read from the ExodusII file
  std::map<SubdomainName, SubdomainID> _block_name_to_id;

  /// Map from block names to block IDs. Read from the ExodusII file
  std::map<SubdomainID, SubdomainName> _block_id_to_name;

private:
  static Threads::spin_mutex _solution_user_object_mutex;
};
