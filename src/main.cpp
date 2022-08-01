#include "esm_freertos_malloc.hpp"
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <chrono>
template <class DT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = {};

public:
    void tick() { 
        _end = timep_t{}; 
        _start = ClockT::now(); 
    }
    
    void tock() { _end = ClockT::now(); }
    
    template <class T = DT> 
    auto duration() const noexcept  { 
        return std::chrono::duration_cast<T>(_end - _start); 
    }
};

// rtos malloc report 
int main(int, char **) {
  Timer clock; 
  esm::FreeRTOSMallocBasic mallocBasic;
  mallocBasic.init();
  clock.tick();
  for (size_t i = 0; i < 2000; i++) {
    unsigned int *tmp;
    unsigned int *ptrs[5000];
    for (size_t j = 0; j < 5000; j++) {
      unsigned int *a = (unsigned int *)mallocBasic.malloc(sizeof(int));
      *a = 0xdadadada;
      ptrs[j] = a;
      if (rand() % 100 > 50) {
        ptrs[j] = a;
      } else {
        mallocBasic.rtos_free(a);
        ptrs[j] = nullptr;
      }
    }
    for (size_t k = 0; k < 5000; k++) {
      if (ptrs[k] != nullptr) {
        mallocBasic.rtos_free(ptrs[k]);
      }
    }
  }
  clock.tock();
  std::cout << "Run time = " << clock.duration().count() << " ms\n";
}


// //system malloc report
// int main(int, char **) {
//   Timer clock; 
//   clock.tick();
//   for (size_t i = 0; i < 2000; i++) {
//     unsigned int *tmp;
//     unsigned int *ptrs[5000];
//     for (size_t j = 0; j < 5000; j++) {
//       unsigned int *a = (unsigned int *)malloc(sizeof(unsigned int));
//       *a = 0xdadadada;
//       ptrs[j] = a;
//       if (rand() % 100 > 50) {
//         ptrs[j] = a;
//       } else {
//         free(a);
//         ptrs[j] = nullptr;
//       }
//     }
//     for (size_t k = 0; k < 5000; k++) {
//       if (ptrs[k] != nullptr) {
//         free(ptrs[k]);
//       }
//     }
//   }
//   clock.tock();
//   std::cout << "Run time = " << clock.duration().count() << " ms\n";
// }

