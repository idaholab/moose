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
#include "Coupleable.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "MaterialProperty.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "quadrature_gauss.h"
#include "elem.h"

// forward declarations
class MooseMesh;
class SubProblemInterface;

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
struct QpData
{
  virtual ~QpData(){}

  inline virtual QpData& operator=(const QpData &) { return *this; }
};


/**
 * Holds material properties that are assigned to blocks.
 */
class Material :
  public MooseObject,
  public Coupleable,
  public TransientInterface,
  public PostprocessorInterface
{
public:
  Material(const std::string & name, InputParameters parameters);
  
  virtual ~Material();

  /** 
   * Block ID the Material is active on.
   * 
   * @return The block ID.
   */
  unsigned int blockID();

  /**
   * Updates the old (first) material properties to the current/new material properties (second)
   */
  void updateDataState();

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

protected:
  Problem & _problem;
  SubProblemInterface & _subproblem;
  THREAD_ID _tid;
  bool _bnd;
  MaterialData & _material_data;

  unsigned int _qp; 

  QBase * & _qrule;
  const std::vector<Real> & _JxW;
  const std::vector< Point > & _q_point;

  const Elem * & _current_elem;

  MooseMesh & _mesh;
  unsigned int _dim;

// struct DeleteFunctor 
//   {
//     void operator()(const std::pair<const unsigned int, MooseMooseArray<QpData *> > & p) const
//     {
//       //for(MooseArray<QpData *>::iterator i = p.second.begin(); i != p.second.end(); ++i)
//       //  delete *i;
//       std::cerr << p.first << " ";
//       //MooseArray<const QpData *>::iterator i = p.second;
//       std::cerr << p.second[0] << std::endl;
//     }
//  };
  
  enum QP_Data_Type { CURR, PREV };

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
   * This function returns a reference to a standard vector of datastructures for all
   * the quadrature points on the current element
   */
  virtual std::vector<QpData *> & getData(QP_Data_Type qp_data_type);

  /**
   * Block ID this material is active on.
   */
  unsigned int _block_id;

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name)
  {
    return _material_data.declareProperty<T>(prop_name);
  }

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name)
  {
    return _material_data.declarePropertyOld<T>(prop_name);
  }

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name)
  {
    return _material_data.declarePropertyOlder<T>(prop_name);
  }

  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;
};


template<>
InputParameters validParams<Material>();

#endif //MATERIAL_H
