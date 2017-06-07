/* Copyright 2017 Battelle Energy Alliance, LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef RANDOMCLASS_H
#define RANDOMCLASS_H

class RandomClassImpl;


/**
 * The RandomClass class allows to create a random number generator instance
 * anywhere in crow
 */

class RandomClass {
  RandomClassImpl *_rng;
  const double _range;
public:
  RandomClass();
  ~RandomClass();
  void seed(unsigned int seed);
  double random();
};

#endif /* RANDOMCLASS_H */
