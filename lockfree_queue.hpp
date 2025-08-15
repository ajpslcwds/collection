#include <atomic>
#include <memory>

template <typename T> class LockFreeQueue
{
  private:
    struct Node
    {
        T data;
        std::atomic<Node *> next;
        Node(const T &value) : data(value), next(nullptr)
        {
        }
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;

  public:
    LockFreeQueue()
    {
        Node *dummy = new Node(T{});
        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue()
    {
        Node *current = head.load();
        while (current)
        {
            Node *next = current->next.load();
            delete current;
            current = next;
        }
    }

    // Push operation for multi-threaded writes
    void push(const T &value)
    {
        Node *new_node = new Node(value); // 创建新节点
        Node *current_tail = nullptr;
        Node *null_node = nullptr;

        while (true)
        {
            current_tail = tail.load(std::memory_order_acquire);             // 获取当前尾节点
            Node *next = current_tail->next.load(std::memory_order_acquire); // 获取尾节点的下一个节点

            if (current_tail == tail.load(std::memory_order_acquire))
            { // 确认尾节点未被修改
                if (next == nullptr)
                { // 尾节点的next为空，说明可以添加新节点
                    if (current_tail->next.compare_exchange_weak(next, new_node, std::memory_order_release,
                                                                 std::memory_order_relaxed))
                    {          // 尝试将next指向新节点
                        break; // 成功添加，退出循环
                    }
                }
                else
                { // 尾节点的next不为空，说明尾节点已落后
                    tail.compare_exchange_weak(current_tail, next, std::memory_order_release,
                                               std::memory_order_relaxed); // 尝试更新tail到next
                }
            }
        }
        tail.compare_exchange_weak(current_tail, new_node, std::memory_order_release,
                                   std::memory_order_relaxed); // 最后更新tail指向新节点
    }

    // Pop operation for multi-threaded reads
    bool pop(T &value)
    {
        while (true)
        {
            Node *current_head = head.load(std::memory_order_acquire);       // 获取当前头节点
            Node *current_tail = tail.load(std::memory_order_acquire);       // 获取当前尾节点
            Node *next = current_head->next.load(std::memory_order_acquire); // 获取头节点的下一个节点

            if (current_head == head.load(std::memory_order_acquire))
            { // 确认头节点未被修改
                if (current_head == current_tail)
                { // 检查队列是否为空
                    if (next == nullptr)
                    {
                        return false; // 队列为空
                    }
                    tail.compare_exchange_weak(current_tail, next, std::memory_order_release,
                                               std::memory_order_relaxed); // 尝试更新tail到next
                }
                else
                {
                    if (next == nullptr)
                    {
                        return false; // 队列为空
                    }
                    value = next->data; // 获取下一个节点的数据
                    if (head.compare_exchange_weak(current_head, next, std::memory_order_release,
                                                   std::memory_order_relaxed))
                    {                        // 尝试将head移到next
                        delete current_head; // 释放旧头节点
                        return true;         // 成功弹出
                    }
                }
            }
        }
    }
};