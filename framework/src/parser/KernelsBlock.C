#include "KernelsBlock.h"

#include "KernelFactory.h"
#include "AuxKernel.h"
#include "TransientBlock.h"

KernelsBlock::KernelsBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{}

void
KernelsBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the KernelsBlock Object\n";
#endif

  // See if there is a transient block and setup time params before calling Kernel::init()
  TransientBlock * t_block = dynamic_cast<TransientBlock *>(locateBlock("Execution/Transient"));
  if (t_block != NULL) 
  {
    t_block->setOutOfOrderTransientParams(Moose::equation_system->parameters);

    Real reject_step_error = Moose::equation_system->parameters.get<Real> ("reject_step_error");
    if (reject_step_error > 0)
    {
      TransientNonlinearImplicitSystem &system =
        Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
      system.add_vector("time_error");
      system.add_vector("old_solution");
    }
  }

  Kernel::init(Moose::equation_system);
  // TODO: Figure out why this can't be called in the AuxKernelsBlock
  AuxKernel::init();
  
  // Add the kernels to the system
  visitChildren();
}  
