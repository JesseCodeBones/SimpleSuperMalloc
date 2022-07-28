#include "esm_freertos_malloc.hpp"
#include <assert.h>
#include <iostream>
#include <mutex>
#include <stdlib.h>
#define MAXLEN 1024

std::mutex freeRTOSMallocMutex;

namespace esm {

FreeRTOSMallocBasic::FreeRTOSMallocBasic(/* args */) {}
FreeRTOSMallocBasic::~FreeRTOSMallocBasic() {
  free(block);
}
int FreeRTOSMallocBasic::init() noexcept {

  uint8_t *mmapResult = (uint8_t *)calloc(MAXLEN * MAXLEN, sizeof(unsigned int));
  block = mmapResult;
  alloc_node_t *new_memory_block =
      (alloc_node_t *)align_up((uintptr_t)mmapResult, sizeof(void *));
  new_memory_block->size = (uintptr_t)mmapResult + MAXLEN * MAXLEN -
                           (uintptr_t)new_memory_block -
                           offsetof(alloc_node_t, block);

  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  list_add(&(new_memory_block->node), &freeList);
  return 0;
}

void *FreeRTOSMallocBasic::malloc(size_t size) noexcept {
  void *ptr = nullptr;
  alloc_node_t *block = nullptr;
  assert(size > 0);
  size = align_up(size, sizeof(void *));
  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  unsigned int off = offsetof(alloc_node_t, node);
  for (alloc_node_t *pos = (alloc_node_t*)((uintptr_t)freeList.next - off);
       &pos->node != &freeList; pos = (alloc_node_t*)((uintptr_t)(pos->node.next) - off)) {
    if (pos->size >= size) { 
      ptr = &pos->block;
      block = pos;
      break;
    }
  }
  if (ptr) {
    // Can we split the block?
    if ((block->size - size) >= MIN_ALLOC_SZ) {
      alloc_node_t *newBlock =
          (alloc_node_t *)((uintptr_t)(&block->block) + size);
      newBlock->size = block->size - size - ALLOC_HEADER_SZ;
      block->size = size;
      list_insert(&newBlock->node, &block->node, block->node.next);
    }
    list_del(&block->node);
  } else {
    std::cout<<"end \n";
  }
  return ptr;
}

void FreeRTOSMallocBasic::rtos_free(void *ptr) noexcept {
  alloc_node_t *currentBlock =
      (alloc_node_t *)((uintptr_t)ptr - offsetof(alloc_node_t, block));
  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  bool alreadyInsert = false;
  for (alloc_node_t *pos = (alloc_node_t*)((uintptr_t)freeList.next - offsetof(alloc_node_t, node));
       &pos->node != &freeList; pos = (alloc_node_t*)((uintptr_t)(pos->node.next) - offsetof(alloc_node_t, node))) {
    if ((uintptr_t)pos > (uintptr_t)currentBlock) {
      list_insert(&currentBlock->node, pos->node.prev, &pos->node);
      alreadyInsert = true;
      break;
    }
  }
  if (!alreadyInsert) {
    list_add_tail(&currentBlock->node, &freeList);
  }
  deflagFreeList();
}

void FreeRTOSMallocBasic::deflagFreeList() {
  alloc_node_t *prevNode = nullptr;
  alloc_node_t *temp = nullptr;
  alloc_node_t *pos = nullptr;

  for (pos = ((alloc_node_t*)((uintptr_t)freeList.next - offsetof(alloc_node_t, node))),
      temp = (alloc_node_t*)((uintptr_t)pos->node.next - offsetof(alloc_node_t, node));
       pos->node.next != &freeList;
       pos = temp, temp = (alloc_node_t*)((uintptr_t)temp->node.next - offsetof(alloc_node_t, node))) {
    if (prevNode) {
      if (((uintptr_t)&prevNode->block + prevNode->size) == (uintptr_t)pos) {
        prevNode->size += ALLOC_HEADER_SZ + pos->size;
        list_del(&pos->node);
        continue;
      }
    }
    prevNode = pos;
  }
}

} // namespace esm