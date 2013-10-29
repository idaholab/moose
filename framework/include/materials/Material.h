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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "MaterialProperty.h"
#include "MaterialPropertyInterface.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"
#include "Problem.h"
#include "SubProblem.h"
#include "DependencyResolverInterface.h"
#include "Function.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "Restartable.h"
#include "Reportable.h"
#include "ZeroInterface.h"
// libMesh includes
#include "libmesh/quadrature_gauss.h"
#include "libmesh/elem.h"

// forward declarations
class Material;
class MooseMesh;

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
struct QpData
{
  virtual ~QpData(){}

  inline virtual QpData& operator=(const QpData &) { return *this; }
};


template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks.
 */
class Material :
  public MooseObject,
  public BlockRestrictable,
  public BoundaryRestrictable,
  public SetupInterface,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public ScalarCoupleable,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  public DependencyResolverInterface,
  public Restartable,
  public Reportable,
  public ZeroInterface
{
public:
  Material(const std::string & name, InputParameters parameters);

  virtual ~Material();

  /**
   * This function is called at the beginning of each timestep
   * for each active material block
   */
  virtual void timeStepSetup();

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties();

  /**
   * Initialize stateful properties (if material has some)
   */
  virtual void initStatefulProperties(unsigned int n_points);

  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

  virtual
  const std::set<std::string> &
  getRequestedItems() { return _depend_props; }

  virtual
  const std::set<std::string> &
  getSuppliedItems() { return _supplied_props; }


protected:
  SubProblem & _subproblem;

  FEProblem & _fe_problem;
  THREAD_ID _tid;
  Assembly & _assembly;
  bool _bnd;
  bool _neighbor;
  MaterialData & _material_data;

  unsigned int _qp;

  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _q_point;
  /// normals at quadrature points (valid only in boundary materials)
  const MooseArray<Point> & _normals;

  const Elem * & _current_elem;

  /// current side of the current element
  unsigned int & _current_side;

  MooseMesh & _mesh;
//  unsigned int _dim;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  std::set<std::string> _depend_props;

  std::set<std::string> _supplied_props;

  enum QP_Data_Type {
    CURR,
    PREV
  };

  /**
   * Initialize stateful properties at quadrature points.  Note when using this function you only need to address
   * the "current" material properties not the old ones directly, i.e. if you have a property named "_diffusivity"
   * and an older property named "_diffusivity_old".  You only need to initialize diffusivity.  MOOSE will use
   * copy that initial value to the old and older values as necessary.
   */
  virtual void initQpStatefulProperties();

  /**
   * Compute material properties in quadrature point
   * Materials has to redefine this.
   */
  virtual void computeQpProperties();

  /**
   * This function is called to create the data structure that will be associated
   * with a quadrature point
   */
  virtual QpData * createData();

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name, bool is_get=false)
  {
    registerPropName(prop_name, is_get);
    return _material_data.declareProperty<T>(prop_name);
  }

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name, bool is_get=false)
  {
    _has_stateful_property = true;
    registerPropName(prop_name, is_get);
    return _material_data.declarePropertyOld<T>(prop_name);
  }

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name, bool is_get=false)
  {
    _has_stateful_property = true;
    registerPropName(prop_name, is_get);
    return _material_data.declarePropertyOlder<T>(prop_name);
  }

  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

private:
  /**
   * Small helper function to call storeMatPropName
   */
  void registerPropName(std::string prop_name, bool is_get)
  {
    // Store material properties for block ids
    for (std::set<SubdomainID>::const_iterator it = _blk_ids.begin(); it != _blk_ids.end(); ++it)
    {
      // Only save this prop as a "supplied" prop is it was registered as a result of a call to declareProperty not getMaterialProperty
      if (!is_get)
        _supplied_props.insert(prop_name);
      _fe_problem.storeMatPropName(*it, prop_name);
      _subproblem.storeMatPropName(*it, prop_name);
    }

    // Store material properites for the boundary ids
    for (std::set<BoundaryID>::const_iterator it = _bnd_ids.begin(); it != _bnd_ids.end(); ++it)
    {
      // \TODO: see ticket #2192
      // Only save this prop as a "supplied" prop is it was registered as a result of a call to declareProperty not getMaterialProperty
      // if (!is_get)
      //  _supplied_props.insert(prop_name);
      _fe_problem.storeMatPropName(*it, prop_name);
      _subproblem.storeMatPropName(*it, prop_name);
    }
  }

  bool _has_stateful_property;
};


template<typename T>
MaterialProperty<T> &
Material::getMaterialProperty(const std::string & name)
{
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the same memory)
  _depend_props.insert(name);
  return declareProperty<T>(name, true);
}

template<typename T>
MaterialProperty<T> &
Material::getMaterialPropertyOld(const std::string & name)
{
  _depend_props.insert(name);
  return declarePropertyOld<T>(name, true);
}

template<typename T>
MaterialProperty<T> &
Material::getMaterialPropertyOlder(const std::string & name)
{
  _depend_props.insert(name);
  return declarePropertyOlder<T>(name, true);
}

#endif //MATERIAL_H
