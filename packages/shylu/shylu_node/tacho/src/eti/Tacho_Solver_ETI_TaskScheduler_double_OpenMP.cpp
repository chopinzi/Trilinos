#include "Kokkos_Core.hpp"

#include "Tacho.hpp"
#include "Tacho_Solver.hpp"
#include "Tacho_Solver_Impl.hpp"

namespace Tacho {
#if defined(KOKKOS_ENABLE_OPENMP)
  using eti_scheduler_type = Kokkos::TaskScheduler<Kokkos::OpenMP>; 
  template struct Solver<double,eti_scheduler_type>;
#endif
}
