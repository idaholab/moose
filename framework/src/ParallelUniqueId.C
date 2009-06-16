#include "ParallelUniqueId.h"

tbb::concurrent_queue<unsigned int> ParallelUniqueId::ids;
