#include "Numerics.h"
#include <sstream>
#include "MooseError.h"

namespace Numerics {

  void* auxpt = NULL;

  Real Newton_Solver(Real x0, Real f0, Real (*fct)(Real x, Real f0), Real (*dfct)(Real x), Real tola, Real tolr, unsigned int n_max)
  {
    unsigned int n=0; // iteration index
    if(x0==0)  x0 = tola;  // avoid setting initial guess to 0; 

    Real res=(*fct)(x0,f0); // store the current residual value
    Real xnew=x0;
    Real xold=x0;

    while (n <= n_max)
    {
      if(std::fabs(res)<tola && std::fabs((xnew-xold)/xold)<tolr) 
      {
        //std::cout<<"  Newton iteration: total iteration "<<n<<" res= "<<res<<std::endl; //debug
        return xnew;
      }

      n++;
      xold=xnew;
      res=(*fct)(xold, f0);
      xnew=xold-res/(*dfct)(xold);
      //std::cout<<"  Newton iteration: n= "<<n<<" xold= "<<xold<<" xnew= "<<xnew<<" res= "<<res<<std::endl; //debug
    }
    
    std::stringstream error_message;
    error_message << "Newton iteration cannot converge after " << (n-1)<<"; final value is "<<xnew;
    //std::cerr << error_message.str() << std::endl;
    mooseError(error_message.str() );
    return 0.;
  }
}

