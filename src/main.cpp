#include "esm_freertos_malloc.hpp"
#include <iostream>
int main(int, char **) {
  esm::FreeRTOSMallocBasic mallocBasic;
  mallocBasic.init();

  for (size_t i = 0; i < 1130; i++) {
    for (size_t j = 0; j < 2220; j++) {
      unsigned int *a = (unsigned int *)mallocBasic.malloc(sizeof(int));
      *a = 0xffffffff;
      mallocBasic.free(a);
    }
  }

  std::cout << "finished \n";
}
