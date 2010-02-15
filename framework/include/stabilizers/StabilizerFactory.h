#ifndef STABILIZERFACTORY_H
#define STABILIZERFACTORY_H

#include "Stabilizer.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef Stabilizer * (*stabilizerBuildPtr)(std::string name,
                                   InputParameters parameters,
                                   std::string var_name,
                                   std::vector<std::string> coupled_to,
                                   std::vector<std::string> coupled_as);
/**
 * Typedef to make things easier.
 */
typedef InputParameters (*stabilizerParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::map<unsigned int, Stabilizer *>::iterator StabilizerIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator StabilizerNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename StabilizerType>
Stabilizer * buildStabilizer(std::string name,
                     InputParameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
{
  return new StabilizerType(name, parameters, var_name, coupled_to, coupled_as);
}

/**
 * Responsible for building Stabilizers on demand and storing them for retrieval
 */
class StabilizerFactory
{
public:
  static StabilizerFactory * instance();

  template<typename StabilizerType> 
  void registerStabilizer(std::string name)
  {
    name_to_build_pointer[name]=&buildStabilizer<StabilizerType>;
    name_to_params_pointer[name]=&validParams<StabilizerType>;
  }

  Stabilizer * add(std::string stabilizer_name,
               std::string name,
               InputParameters parameters,
               std::string var_name,
               std::vector<std::string> coupled_to=std::vector<std::string>(0),
               std::vector<std::string> coupled_as=std::vector<std::string>(0));
  

  Stabilizer * add(std::string stabilizer_name,
               std::string name,
               InputParameters parameters,
               std::string var_name,
               std::vector<std::string> coupled_to,
               std::vector<std::string> coupled_as,
               unsigned int block_id);
  
  InputParameters getValidParams(std::string name);

  bool isStabilized(unsigned int var_num);
  
  StabilizerIterator activeStabilizersBegin(THREAD_ID tid);
  StabilizerIterator activeStabilizersEnd(THREAD_ID tid);

  StabilizerIterator blockStabilizersBegin(THREAD_ID tid, unsigned int block_id);
  StabilizerIterator blockStabilizersEnd(THREAD_ID tid, unsigned int block_id);

  bool activeStabilizerBlocks(std::set<subdomain_id_type> & set_buffer) const;

  StabilizerNamesIterator registeredStabilizersBegin();
  StabilizerNamesIterator registeredStabilizersEnd();
  
private:
  StabilizerFactory();

  virtual ~StabilizerFactory();
  
  std::map<std::string, stabilizerBuildPtr> name_to_build_pointer;
  std::map<std::string, stabilizerParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_stabilizer_names;
  std::vector<std::map<unsigned int, Stabilizer *> > active_stabilizers;

  std::vector<std::map<unsigned int, std::map<unsigned int, Stabilizer *> > > block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;
};

#endif //STABILIZERFACTORY_H
