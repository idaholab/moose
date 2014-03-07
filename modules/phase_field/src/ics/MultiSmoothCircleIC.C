#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<MultiSmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleIC>();
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to be placed on GB");
  params.addRequiredParam<Real>("bubspac", "minimum spacing of bubbles, measured from center to center");
  params.addRequiredParam<Real>("Lx", "length of simulation domain in x-direction");
  params.addRequiredParam<Real>("Ly", "length of simulation domain in y-direction");
  params.addParam<Real>("Lz",0.0,"length of simulation domain in z-direction");
  params.addParam<unsigned int>("rand_seed",2000,"random seed");
  params.addParam<unsigned int>("numtries", 1000,"The number of tries");
  params.addParam<Real>("radius_variation",0.0,"Plus or minus Percent of random variation in the bubble radius");
  //These are SmoothCircleIC inputs that are not needed here.
  params.set<Real>("x1") = 0.0;
  params.set<Real>("y1") = 0.0;

  return params;
}

MultiSmoothCircleIC::MultiSmoothCircleIC(const std::string & name,
                                               InputParameters parameters)
  :SmoothCircleIC(name, parameters),
   _numbub(getParam<unsigned int>("numbub")),
   _bubspac(getParam<Real>("bubspac")),
   _Lx(getParam<Real>("Lx")),
   _Ly(getParam<Real>("Ly")),
   _Lz(getParam<Real>("Lz")),
   _rnd_seed(getParam<unsigned int>("rand_seed")),
   _numtries(getParam<unsigned int>("numtries")),
   _radius_variation(getParam<Real>("radius_variation"))
{
}

void
MultiSmoothCircleIC::initialSetup()
{
   _bubcent.resize(_numbub);
   _bubradi.resize(_numbub);

   MooseRandom::seed(_rnd_seed);
   for(unsigned int i=0; i<_numbub; i++)
     {
       //Vary bubble radius
       _bubradi[i] = _radius*(1.0 + (1.0 - 2.0*MooseRandom::rand())*_radius_variation);
       if (_bubradi[i] < 0.0)
         _bubradi[i] = 0.0;

       //Vary circle positions
       unsigned int num_tries = 0;

       Real rr = 0.0;
       Real xx, yy, zz;

       while (rr < _bubspac && num_tries < _numtries)
       {
         num_tries++;
         //Moose::out<<"num_tries: "<<num_tries<<std::endl;

         Real ran1 = MooseRandom::rand();
         Real ran2 = MooseRandom::rand();
         Real ran3 = MooseRandom::rand();

         xx = ran1*(_Lx - _bubspac) + 0.5*_bubspac;
         yy = ran2*(_Ly - _bubspac) + 0.5*_bubspac;

         if (_Lz == 0.0)
           zz = 0.0;
         else
           zz = ran3*(_Lz - _bubspac) + 0.5*_bubspac;

         for(unsigned int j=0; j<i; j++)
         {
           if (j==0)
             rr = 1000.0;

           Real rx = abs(xx-_bubcent[j](0));
           Real ry = abs(yy-_bubcent[j](1));
           Real rz = abs(zz-_bubcent[j](2));
           Real tmp_rr = std::sqrt(rx*rx + ry*ry + rz*rz);
           if (tmp_rr < rr)
             rr = tmp_rr;
         }
         if (i==0)
           rr = _Lx;

       }

       if (num_tries == _numtries)
       {
         Moose::out<<"Too many tries in MultiSmoothCircle IC "<<std::endl;
         mooseError("Toom many tried in MultiSmoothCircleIC");
       }


       _bubcent[i](0) =   xx;
       _bubcent[i](1) =   yy;
       _bubcent[i](2) =   zz;
     }
}

Real
MultiSmoothCircleIC::value(const Point & p)
{
  Real val = _outvalue;
  Real val2 = 0.0;
  for(unsigned int i=0; i<_numbub; i++)
    {
      _radius = _bubradi[i];
      _center = _bubcent[i];

      if((p-_center).size() < _radius + _int_width/2.0)
      {
        val2 = SmoothCircleIC::value(p);
        if (val2 > val)
          val = val2;
      }
    }
  return val;
}

RealGradient
MultiSmoothCircleIC::gradient(const Point & p)
{
  RealGradient grad = Gradient(0.0,0.0,0.0);
  RealGradient grad2;

  for(unsigned int i=0; i<_numbub; i++)
    {
      _radius = _bubradi[i];
      _center = _bubcent[i];
      if((p-_center).size() < _radius + _int_width/2.0)
      {
        grad2 = SmoothCircleIC::gradient(p)/_numbub;
        //if (grad.size() < grad2.size())
          grad = grad2;
      }
    }

  return grad;
}
