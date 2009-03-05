#include "Kernel.h"
#include "QpData.h"

#ifndef MATERIAL_H
#define MATERIAL_H

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
           std::vector<std::string> coupled_as)
    :Kernel(name, parameters, Kernel::_es->get_system(0).variable_name(0), false, coupled_to, coupled_as),
    _zero(0),
    _grad_zero(0),
    _block_id(block_id)
  {}

  virtual ~Material(){}

  /** 
   * Block ID the Material is active on.
   * 
   * @return The block ID.
   */
  unsigned int blockID(){ return _block_id; }

  /**
   * Causes the material to recompute all of it's values
   * at the quadrature points.  This is a helper in the base
   * class that does a bunch of common setup first then calls
   * computeProperties().
   */
  void materialReinit();

  /**
   * Retrieve the Real valued property named "name"
   */
  std::vector<Real> & getRealProperty(const std::string & name)
  {
    std::map<std::string, std::vector<Real> >::iterator it = _real_props.find(name);

    if(it != _real_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
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
  }

  /**
   * Retrieve the Tensor valued property named "name"
   */
  std::vector<std::vector<std::vector<Real> > > & getTensorProperty(const std::string & name)
  {
    std::map<std::string, std::vector<std::vector<std::vector<Real> > > >::iterator it = _tensor_props.find(name);

    if(it != _tensor_props.end())
      return it->second;

    std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  }

  /**
   * Updates the old (first) material properties to the current/new material properies (second)
   */
  void updateDataState();

protected:
  std::vector<Real> _zero;
  std::vector<RealGradient> _grad_zero;

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties() = 0;

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
  std::vector<std::vector<std::vector<Real> > > & declareTensorProperty(const std::string & name)
  {
    return _tensor_props[name];
  }

  std::map<std::string, std::vector<Real> > _real_props;
  std::map<std::string, std::vector<std::vector<Real> > > _vector_props;
  std::map<std::string, std::vector<std::vector<std::vector<Real> > > > _tensor_props;
  std::map<unsigned int, std::pair<QpData *, QpData *> > _qp_props;
};

#endif //MATERIAL_H
