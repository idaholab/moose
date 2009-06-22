#ifndef MATERIAL_H
#define MATERIAL_H

#include "Kernel.h"
#include "QpData.h"

//libmesh includes
#include "tensor_value.h"


/**
 * Holds material properties that are assigned to blocks.
 */
class Material : public Kernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  Material(std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as);
  
  virtual ~Material()
  {
    // TODO: Implement destructor to clean up after the _qp_prev and _qp_curr data objects
    
    //std::for_each(_qp_prev.begin(), _qp_prev.end(), DeleteFunctor());
    //std::for_each(_qp_curr.begin(), _qp_curr.end(), DeleteFunctor());
  }

  /** 
   * Block ID the Material is active on.
   * 
   * @return The block ID.
   */
  unsigned int blockID();

  /**
   * Causes the material to recompute all of it's values
   * at the quadrature points.  This is a helper in the base
   * class that does a bunch of common setup first then calls
   * computeProperties().
   */
  void materialReinit();

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealProperty(const std::string & name)
  {
    std::map<std::string, Real >::iterator it = _constant_real_props.find(name);

    if(it != _constant_real_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Retrieve the Real valued property named "name"
   */
  std::vector<Real> & getRealProperty(const std::string & name)
  {
    std::map<std::string, std::vector<Real> >::iterator it = _real_props.find(name);

    if(it != _real_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Retrieve the Gradient valued property named "name"
   */
  std::vector<RealGradient> & getGradientProperty(const std::string & name)
  {
    std::map<std::string, std::vector<RealGradient> >::iterator it = _gradient_props.find(name);

    if(it != _gradient_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Retrieve the Vector valued property named "name"
   */
  std::vector<std::vector<Real> > & getVectorProperty(const std::string & name)
  {
    std::map<std::string, std::vector<std::vector<Real> > >::iterator it = _vector_props.find(name);

    if(it != _vector_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Retrieve the Tensor valued property named "name"
   */
  std::vector<RealTensorValue> & getTensorProperty(const std::string & name)
  {
    std::map<std::string, std::vector<RealTensorValue> >::iterator it = _tensor_props.find(name);

    if(it != _tensor_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Retrieve the Matrix valued property named "name"
   */
  std::vector<std::vector<std::vector<Real> > > & getMatrixProperty(const std::string & name)
  {
    std::map<std::string, std::vector<std::vector<std::vector<Real> > > >::iterator it = _matrix_props.find(name);

    if(it != _matrix_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
    libmesh_error();
  }

  /**
   * Updates the old (first) material properties to the current/new material properies (second)
   */
  void updateDataState();

protected:
// struct DeleteFunctor 
//   {
//     void operator()(const std::pair<const unsigned int, std::vector<QpData *> > & p) const
//     {
//       //for(std::vector<QpData *>::iterator i = p.second.begin(); i != p.second.end(); ++i)
//       //  delete *i;
//       std::cerr << p.first << " ";
//       //std::vector<const QpData *>::iterator i = p.second;
//       std::cerr << p.second[0] << std::endl;
//     }
//  };
  
  enum QP_Data_Type { CURR, PREV };

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties() = 0;

  /**
   * This function is called to create the data structure that will be associated
   * with a quadrature point
   */
  virtual QpData * createData() { return NULL; }

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
   * Doesn't do anything for materials.
   */
  virtual Real computeQpResidual(){ return 0; }

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<Real> & declareRealProperty(const std::string & name)
  {
    return _real_props[name];
  }
  
  /**
   * Declare the Constant Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  Real & declareConstantRealProperty(const std::string & name)
  {
    return _constant_real_props[name];
  }

  /**
   * Declare the Gradient valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<RealGradient> & declareGradientProperty(const std::string & name)
  {
    return _gradient_props[name];
  }

  /**
   * Declare the Vector valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<std::vector<Real> > & declareVectorProperty(const std::string & name)
  {
    return _vector_props[name];
  }

  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<RealTensorValue> & declareTensorProperty(const std::string & name)
  {
    return _tensor_props[name];
  }

  /**
   * Declare the Matrix valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<std::vector<std::vector<Real> > > & declareMatrixProperty(const std::string & name)
  {
    return _matrix_props[name];
  }

  std::map<std::string, Real > _constant_real_props;
  std::map<std::string, std::vector<Real> > _real_props;
  std::map<std::string, std::vector<RealGradient> > _gradient_props;
  std::map<std::string, std::vector<std::vector<Real> > > _vector_props;
  std::map<std::string, std::vector<RealTensorValue> > _tensor_props;
  std::map<std::string, std::vector<std::vector<std::vector<Real> > > > _matrix_props;
  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

};

#endif //MATERIAL_H
