#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <utility>

template <typename T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
 public:
  using InitFunc = std::function<void(T*)>;
  using ObjectPoolPtr = std::shared_ptr<ObjectPool<T>>;

  // 使用 T 的构造参数初始化每个对象
  template <typename... Args>
  explicit ObjectPool(uint32_t num_objects, Args&&... args)
      : num_objects_(num_objects) {
    allocate_and_construct(std::forward<Args>(args)...);
  }

  // 使用额外的 InitFunc 在每个对象构造后进行初始化（例如重置成员）
  template <typename... Args>
  ObjectPool(uint32_t num_objects, InitFunc f, Args&&... args)
      : num_objects_(num_objects) {
    allocate_and_construct(std::forward<Args>(args)..., f);
  }

  virtual ~ObjectPool() {
    // 析构每个 Node（注意：当析构被调用时，应该没有外部持有的对象，
    // 因为每个外部 shared_ptr 持有池的 shared_ptr，从而延长池的寿命）
    const size_t size = sizeof(Node);
    for (uint32_t i = 0; i < num_objects_; ++i) {
      Node* node = reinterpret_cast<Node*>(object_arena_ + i * size);
      node->~Node();
    }
    ::operator delete(static_cast<void*>(object_arena_),
                      num_objects_ * sizeof(Node));
    object_arena_ = nullptr;
    free_head_ = nullptr;
  }

  // 获取对象：返回 shared_ptr<T>，当 shared_ptr 被销毁时对象会回到池
  std::shared_ptr<T> GetObject() {
    std::lock_guard<std::mutex> lk(mutex_);
    if (!free_head_) {
      return std::shared_ptr<T>();  // 返回空指针（也可以选择抛出）
    }

    Node* node = free_head_;
    free_head_ = node->next;

    // 创建一个 shared_ptr<ObjectPool> 来保证池在对象外部存活期间不会析构
    auto self = this->shared_from_this();

    // 用自定义 deleter；捕获 node 指针与 shared_ptr<ObjectPool>
    std::shared_ptr<T> sp(&node->object, [self, node](T* /*p*/) {
      // deleter：把 node 放回空闲链表
      self->release_node(node);
    });

    return sp;
  }

  // 禁止拷贝
  ObjectPool(ObjectPool&) = delete;
  ObjectPool& operator=(ObjectPool&) = delete;

 private:
  // Node：先放置 T，再放 next；这样 &node->object == node 地址的开头
  struct Node {
    T object;
    Node* next;

    template <typename... Args>
    Node(Node* nxt, Args&&... args)
        : object(std::forward<Args>(args)...), next(nxt) {}
  };

  uint32_t num_objects_ = 0;
  char* object_arena_ = nullptr;
  Node* free_head_ = nullptr;
  std::mutex mutex_;

  // 将 node 放回空闲链表（由 deleter 调用）
  void release_node(Node* node) {
    std::lock_guard<std::mutex> lk(mutex_);
    node->next = free_head_;
    free_head_ = node;
  }

  // 辅助：分配 arena 并在每个槽内 placement-new Node
  template <typename... Args>
  void allocate_and_construct(Args&&... args) {
    const size_t size = sizeof(Node);
    // 使用 ::operator new 以确保满足对齐要求并能传回给 ::operator delete
    object_arena_ = static_cast<char*>(::operator new(num_objects_ * size));
    if (object_arena_ == nullptr) {
      throw std::bad_alloc();
    }

    free_head_ = nullptr;
    // 构造 Node：Node 的 ctor 会构造内部的 T
    for (int i = 0; i < static_cast<int>(num_objects_); ++i) {
      char* slot = object_arena_ + i * size;
      Node* node = new (slot) Node(free_head_, std::forward<Args>(args)...);
      free_head_ = node;
    }
  }
};
