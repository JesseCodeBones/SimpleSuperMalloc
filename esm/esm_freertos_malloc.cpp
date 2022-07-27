#include "esm_freertos_malloc.hpp"
#include <assert.h>
#include <iostream>
#include <mutex>
#define MAXLEN 1024

std::mutex freeRTOSMallocMutex;

namespace esm {

FreeRTOSMallocBasic::FreeRTOSMallocBasic(/* args */) {}
FreeRTOSMallocBasic::~FreeRTOSMallocBasic() {}
int FreeRTOSMallocBasic::init() noexcept {

  uint8_t mmapResult[MAXLEN];
  alloc_node_t *new_memory_block =
      (alloc_node_t *)align_up((uintptr_t)mmapResult, sizeof(void *));
  new_memory_block->size = (uintptr_t)mmapResult + MAXLEN -
                           (uintptr_t)new_memory_block -
                           offsetof(alloc_node_t, block);

  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  list_add(&new_memory_block->node, &freeList);
  return 0;
}

void *FreeRTOSMallocBasic::malloc(size_t size) noexcept {
  void *ptr = nullptr;
  alloc_node_t *block = nullptr;
  assert(size > 0);
  size = align_up(size, sizeof(void *));
  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  for (alloc_node_t *pos = (alloc_node_t *)freeList.next;
       &pos->node != &freeList; pos = (alloc_node_t *)pos->node.next) {
    if (pos->size > size) { // TODO do not use first hit
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
  }
  return ptr;
}

void FreeRTOSMallocBasic::free(void *ptr) noexcept{
  alloc_node_t *currentBlock =
      (alloc_node_t *)((uintptr_t)ptr - offsetof(alloc_node_t, block));
  std::lock_guard<std::mutex> guard(freeRTOSMallocMutex);
  bool alreadyInsert = false;
  for (alloc_node_t *pos = (alloc_node_t *)freeList.next;
       &pos->node != &freeList; pos = (alloc_node_t *)pos->node.next) {
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
    for (alloc_node_t *pos = (alloc_node_t *)freeList.next;
       &pos->node != &freeList; pos = (alloc_node_t *)pos->node.next) {
    if (prevNode)
    {
        if (((uintptr_t)&prevNode->block + prevNode->size) == (uintptr_t)pos)
        {
            prevNode->size += ALLOC_HEADER_SZ + pos->size;
            alloc_node_t *delnode = pos;
            pos = (alloc_node_t *)pos->node.next;
            list_del(&delnode->node);
            continue;
        }
    } else {
        prevNode = pos;
    }
  }
}
    
} // namespace esm