#include "KernelFactory.h"

KernelFactory *
KernelFactory::instance()
{
  static KernelFactory * instance;
  if(!instance)
    instance=new KernelFactory;
    
  return instance;
}

Kernel *
KernelFactory::add(std::string kernel_name,
                   std::string name,
                   InputParameters parameters,
                   std::string var_name,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as)
{
  Kernel * kernel;
    
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    all_kernels[tid].push_back(kernel);
  }

  return kernel;
}

Kernel *
KernelFactory::add(std::string kernel_name,
                   std::string name,
                   InputParameters parameters,
                   std::string var_name,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as,
                   unsigned int block_id)
{
  Kernel * kernel;
    
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;
      
    kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    all_block_kernels[tid][block_id].push_back(kernel);
  }

  return kernel;
}

InputParameters
KernelFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Kernel "<<std::endl<<std::endl;
    mooseError("");
  }
  
  InputParameters params = name_to_params_pointer[name]();

  if(!params.have_parameter<Real>("start_time"))
    params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that this kernel will be active after.");

  if(!params.have_parameter<Real>("stop_time"))
    params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which this kernel will no longer be active.");

  return params;
}

KernelIterator
KernelFactory::activeKernelsBegin(THREAD_ID tid)
{
  return active_kernels[tid].begin();
}

KernelIterator
KernelFactory::activeKernelsEnd(THREAD_ID tid)
{
  return active_kernels[tid].end();
}


KernelIterator
KernelFactory::blockKernelsBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].begin();
}

KernelIterator
KernelFactory::blockKernelsEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].end();
}

bool
KernelFactory::activeKernelBlocks(std::set<subdomain_id_type> & set_buffer) const
{
  std::map<unsigned int, std::vector<Kernel *> >::const_iterator curr, end;
  end = block_kernels[0].end();

  try 
  {
    for (curr = block_kernels[0].begin(); curr != end; ++curr)
      set_buffer.insert(subdomain_id_type(curr->first));
  }
  catch (std::exception &e)
  {
    mooseError("Invalid block specified in input file");
  }

  // return a boolean indicated whether there are any global kernels active
  return ! active_kernels[0].empty();
}

KernelNamesIterator
KernelFactory::registeredKernelsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_kernel_names.clear();
  _registered_kernel_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, kernelParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_kernel_names.push_back(i->first);
  }
  
  return _registered_kernel_names.begin();
}

KernelNamesIterator
KernelFactory::registeredKernelsEnd()
{
  return _registered_kernel_names.end();
}

void
KernelFactory::updateActiveKernels(THREAD_ID tid)
{
  {
    Real t = Kernel::_t;

    if(t >= 1.0)
    {
      t++;
    }
    
    
    active_kernels[tid].clear();

    KernelIterator all_it = all_kernels[tid].begin();
    KernelIterator all_end = all_kernels[tid].end();
  
    for(; all_it != all_end; ++all_it)
      if((*all_it)->startTime() <= Kernel::_t + (1e-6 * Kernel::_dt) && (*all_it)->stopTime() >= Kernel::_t + (1e-6 * Kernel::_dt))
        active_kernels[tid].push_back(*all_it);
  }
  
  {
    block_kernels[tid].clear();

    std::map<unsigned int, std::vector<Kernel *> >::iterator block_it = all_block_kernels[tid].begin();
    std::map<unsigned int, std::vector<Kernel *> >::iterator block_end = all_block_kernels[tid].end();

    for(; block_it != block_end; ++block_it)
    {
      unsigned int block_num = block_it->first;
      block_kernels[tid][block_num].clear();
      
      KernelIterator all_block_it = block_it->second.begin();
      KernelIterator all_block_end = block_it->second.end();

      for(; all_block_it != all_block_end; ++all_block_it)
        if((*all_block_it)->startTime() <= Kernel::_t + (1e-6 * Kernel::_dt) && (*all_block_it)->stopTime() >= Kernel::_t + (1e-6 * Kernel::_dt))
          block_kernels[tid][block_num].push_back(*all_block_it);
    }
  }
}

KernelFactory::KernelFactory()
{
  active_kernels.resize(libMesh::n_threads());
  all_kernels.resize(libMesh::n_threads());
  block_kernels.resize(libMesh::n_threads());
  all_block_kernels.resize(libMesh::n_threads());
}
  
KernelFactory:: ~KernelFactory() 
{
  {
    std::map<std::string, kernelBuildPtr>:: iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, kernelParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
     
  {
        
    std::vector<std::vector<Kernel *> >::iterator i;
    for (i=active_kernels.begin(); i!=active_kernels.end(); ++i)
    { 

      KernelIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<Kernel *> > >::iterator i;
    for (i=block_kernels.begin(); i!=block_kernels.end(); ++i)
    {
          
      std::map<unsigned int, std::vector<Kernel *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        KernelIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }
}

