/**
 * Mediator Pattern (中介者模式)
 *
 * Intent: 用一个中介对象封装一组对象之间的交互，使各对象不需要显式地
 * 相互引用，从而使其耦合松散。
 *
 * C++17 Features Used:
 *   - std::variant: 类型安全的事件消息容器
 *   - std::visit: 编译期多态事件分发
 *   - std::optional: 可选返回值
 *   - if constexpr: 编译期条件分支
 *   - structured bindings: 简化事件字段解构
 *   - inline variables: 全局常量定义
 *   - std::shared_ptr / std::weak_ptr: 生命周期管理
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ============================================================================
// 事件定义
// ============================================================================

/**
 * 文本消息事件：用户发送的普通聊天文本。
 */
struct TextMessage
{
    std::string sender;
    std::string content;
};

/**
 * 状态变更事件：用户上线或下线。
 */
struct StatusChange
{
    std::string user;
    bool online;  // true = 上线, false = 下线
};

/**
 * 私聊请求事件：一个用户向另一个用户发起私聊。
 */
struct PrivateChatRequest
{
    std::string from;
    std::string to;
};

// 所有可能的事件类型 —— 使用 variant 实现类型安全的联合
using ChatEvent = std::variant<TextMessage, StatusChange, PrivateChatRequest>;

inline constexpr std::string_view kMediatorName = "ChatRoomMediator";

// ============================================================================
// 前置声明
// ============================================================================

class Mediator;

// ============================================================================
// 同事基类 (Colleague)
// ============================================================================

/**
 * 所有参与者的抽象基类。每个参与者持有中介者的 weak_ptr，
 * 通过它来发送事件，而不是直接持有其他参与者的引用。
 */
class Colleague : public std::enable_shared_from_this<Colleague>
{
public:
    explicit Colleague(std::string name)
        : name_(std::move(name))
    {
    }

    virtual ~Colleague() = default;

    [[nodiscard]] std::string_view GetName() const
    {
        return name_;
    }

    /** 绑定中介者（注册阶段由中介者调用）。 */
    void SetMediator(std::shared_ptr<Mediator> mediator)
    {
        mediator_ = std::move(mediator);
    }

    /** 向中介者发送事件。 */
    void Send(const ChatEvent& event);

    /** 接收并处理来自中介者转发的事件（子类重写）。 */
    virtual void Receive(const ChatEvent& event) = 0;

protected:
    std::string name_;
    std::weak_ptr<Mediator> mediator_;
};

// ============================================================================
// 中介者接口 (Mediator)
// ============================================================================

/**
 * 中介者抽象接口，定义了注册参与者和转发事件的能力。
 */
class Mediator : public std::enable_shared_from_this<Mediator>
{
public:
    virtual ~Mediator() = default;

    /** 注册一个同事参与者。 */
    virtual void Register(std::shared_ptr<Colleague> colleague) = 0;

    /** 接收来自某个同事的事件并转发给其他相关同事。 */
    virtual void Notify(const Colleague* sender, const ChatEvent& event) = 0;
};

// ============================================================================
// Colleague::Send 的实现（需要 Mediator 的完整定义）
// ============================================================================

void Colleague::Send(const ChatEvent& event)
{
    if (auto med = mediator_.lock())
    {
        med->Notify(this, event);
    }
    else
    {
        std::cout << "  [WARN] " << name_
                  << " 的中介者已失效，无法发送消息。\n";
    }
}

// ============================================================================
// 具体中介者：聊天室 (ChatRoom)
// ============================================================================

/**
 * 聊天室中介者，管理所有参与者，根据事件类型决定如何转发。
 *
 * 使用 std::visit 对 variant 进行编译期多态分发，
 * 替代传统的 switch-case + 枚举方式。
 */
class ChatRoom : public Mediator
{
public:
    void Register(std::shared_ptr<Colleague> colleague) override
    {
        colleague->SetMediator(shared_from_this());

        // 广播上线通知
        StatusChange status{std::string(colleague->GetName()), true};
        for (auto& c : colleagues_)
        {
            c->Receive(status);
        }

        colleagues_.push_back(std::move(colleague));
    }

    void Notify(const Colleague* sender, const ChatEvent& event) override
    {
        // 使用 std::visit 进行编译期类型分发
        std::visit(
            [this, sender](const auto& e)
            {
                HandleEvent(sender, e);
            },
            event);
    }

private:
    /**
     * 处理 TextMessage —— 广播给除发送者之外的所有人。
     */
    void HandleEvent(const Colleague* /*sender*/, const TextMessage& msg)
    {
        for (auto& c : colleagues_)
        {
            if (c->GetName() != msg.sender)
            {
                c->Receive(msg);
            }
        }
    }

    /**
     * 处理 StatusChange —— 广播给所有人。
     */
    void HandleEvent(const Colleague* /*sender*/, const StatusChange& status)
    {
        for (auto& c : colleagues_)
        {
            if (c->GetName() != status.user)
            {
                c->Receive(status);
            }
        }
    }

    /**
     * 处理 PrivateChatRequest —— 仅转发给目标用户。
     * 使用 std::optional 表示查找结果。
     */
    void HandleEvent(const Colleague* /*sender*/,
                     const PrivateChatRequest& req)
    {
        auto target = FindColleague(req.to);
        if (target.has_value())
        {
            target.value()->Receive(req);
        }
        else
        {
            std::cout << "  [MEDIATOR] 用户 " << req.to << " 不在线，"
                      << "私聊请求无法送达。\n";
        }
    }

    /**
     * 按名字查找同事，返回 optional 以避免空指针。
     */
    [[nodiscard]] std::optional<std::shared_ptr<Colleague>>
    FindColleague(std::string_view name) const
    {
        for (const auto& c : colleagues_)
        {
            if (c->GetName() == name)
            {
                return c;
            }
        }
        return std::nullopt;
    }

    std::vector<std::shared_ptr<Colleague>> colleagues_;
};

// ============================================================================
// 具体同事：普通用户 (User)
// ============================================================================

/**
 * 普通聊天用户。接收事件后根据类型打印不同信息。
 *
 * 使用 if constexpr 分支编译期优化事件处理代码路径。
 */
class User : public Colleague
{
public:
    using Colleague::Colleague;

    void Receive(const ChatEvent& event) override
    {
        std::visit(
            [this](const auto& e)
            {
                HandleEvent(e);
            },
            event);
    }

private:
    /** 默认处理：编译期分发各事件类型 */
    template <typename T>
    void HandleEvent(const T& event)
    {
        if constexpr (std::is_same_v<T, TextMessage>)
        {
            // 结构化绑定，直接解构
            auto [sender, content] = event;
            std::cout << "  [" << name_ << " 收到] " << sender
                      << ": " << content << "\n";
        }
        else if constexpr (std::is_same_v<T, StatusChange>)
        {
            auto [user, online] = event;
            std::string_view action = online ? "上线了" : "下线了";
            std::cout << "  [" << name_ << " 收到通知] " << user
                      << " " << action << "\n";
        }
        else if constexpr (std::is_same_v<T, PrivateChatRequest>)
        {
            auto [from, to] = event;
            std::cout << "  [" << name_ << " 收到私聊请求] 来自 "
                      << from << "\n";
        }
    }
};

// ============================================================================
// main: 使用示例
// ============================================================================

int main()
{
    std::cout << "=== Mediator Pattern (中介者模式) Demo ===\n\n";

    // 创建中介者
    auto chat_room = std::make_shared<ChatRoom>();

    // 创建同事对象
    auto alice = std::make_shared<User>("Alice");
    auto bob = std::make_shared<User>("Bob");
    auto charlie = std::make_shared<User>("Charlie");

    // --- 注册阶段 ---
    std::cout << "[注册 Alice]\n";
    chat_room->Register(alice);

    std::cout << "\n[注册 Bob]\n";
    chat_room->Register(bob);

    std::cout << "\n[注册 Charlie]\n";
    chat_room->Register(charlie);

    // --- 广播消息 ---
    std::cout << "\n--- Alice 发送广播消息 ---\n";
    alice->Send(TextMessage{"Alice", "大家好，今天天气不错！"});

    // --- 私聊请求 ---
    std::cout << "\n--- Bob 发起与 Charlie 的私聊 ---\n";
    bob->Send(PrivateChatRequest{"Bob", "Charlie"});

    // --- 私聊请求：目标不存在 ---
    std::cout << "\n--- Alice 向不存在的用户发起私聊 ---\n";
    alice->Send(PrivateChatRequest{"Alice", "Dave"});

    // --- 状态变更广播 ---
    std::cout << "\n--- Bob 主动广播下线 ---\n";
    StatusChange bob_offline{"Bob", false};
    // Bob 直接向中介者发送状态变更事件
    chat_room->Notify(bob.get(), bob_offline);

    // --- Charlie 继续发送消息 ---
    std::cout << "\n--- Charlie 发送消息（Bob 已下线但仍在列表中）---\n";
    charlie->Send(TextMessage{"Charlie", "Bob 走了？那我一个人说了算！"});

    std::cout << "\n=== Demo 结束 ===\n";
    return 0;
}
