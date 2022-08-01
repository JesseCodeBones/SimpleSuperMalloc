
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>

typedef struct ll_head {
  ll_head *prev;
  ll_head *next;
} ll_t;

typedef struct {
  ll_t node;
  size_t size;
  char *block;
} alloc_node_t;

#define align_up(num, align) (((num) + ((align)-1)) & ~((align)-1))

#define ALLOC_HEADER_SZ offsetof(alloc_node_t, block)

// We are enforcing a minimum allocation size of 8B.
#define MIN_ALLOC_SZ ALLOC_HEADER_SZ + 8

#define ll_head_INIT(name) \
	{                      \
		&(name), &(name)   \
	}
#define LIST_INIT(name) struct ll_head name = ll_head_INIT(name)
namespace esm {

  

class FreeRTOSMallocBasic {
private:
  void deflagFreeList();
  LIST_INIT(freeList);
  uint8_t *block;
  uint8_t mergeHit = 0U;

  static inline void list_insert(struct ll_head *n, struct ll_head *prev,
                                 struct ll_head *next) {
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
  }

  static inline void list_add(struct ll_head *n, struct ll_head *head) {
    // assert((uintptr_t)(n->next) > ((uintptr_t)(head->next) - 1024)  );
    // assert((uintptr_t)(n->next) < ((uintptr_t)(head->next) + 1024)  );
    list_insert(n, head, head->next);
  }

  static inline void list_add_tail(struct ll_head *n, struct ll_head *head) {
    list_insert(n, head->prev, head);
  }
  static inline void list_join_nodes(struct ll_head *prev,
                                     struct ll_head *next) {
    next->prev = prev;
    prev->next = next;
  }
  static inline void list_del(struct ll_head *entry) {
    list_join_nodes(entry->prev, entry->next);
    entry->next = nullptr;
    entry->prev = nullptr;
  }

public:
  FreeRTOSMallocBasic();
  ~FreeRTOSMallocBasic();
  int init() noexcept; // 0 success, others error
  void *malloc(size_t size) noexcept;
  void rtos_free(void* ptr) noexcept;
};

} // namespace esm
