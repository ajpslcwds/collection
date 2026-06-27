/**
 * Bridge Pattern (桥接模式)
 *
 * Intent: Decouple an abstraction from its implementation so that the two
 * can vary independently.
 *
 * C++17 features used:
 *   - std::unique_ptr for ownership semantics
 *   - std::variant + std::visit for compile-time polymorphism alternative
 *   - std::string_view to avoid unnecessary string copies
 *   - inline constexpr for constant definitions
 *   - [[nodiscard]] attribute to prevent ignoring return values
 *   - Structured bindings
 *   - if constexpr for compile-time branching
 */

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// =============================================================================
// Implementation Hierarchy (实现层)
//
// This layer defines the low-level sending interface. Concrete implementations
// provide different transport mechanisms (Email, SMS, Slack). The abstraction
// layer does not know which concrete implementation it is talking to.
// =============================================================================

/// @brief Abstract interface for message transport.
class MessageSender
{
 public:
  virtual ~MessageSender() = default;

  /// Send a raw message through the transport.
  [[nodiscard]] virtual bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const = 0;

  /// Human-readable name of the transport.
  [[nodiscard]] virtual std::string_view TransportName() const noexcept = 0;
};

/// @brief Sends messages via Email.
class EmailSender : public MessageSender
{
 public:
  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const override
  {
    std::cout << "  [Email] From: " << from
              << " | To: " << to
              << " | Subject: " << subject << "\n"
              << "    Body: " << body << "\n";
    return true;
  }

  [[nodiscard]] std::string_view TransportName() const noexcept override
  {
    return "Email";
  }
};

/// @brief Sends messages via SMS.
class SmsSender : public MessageSender
{
 public:
  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view /*subject*/,
      std::string_view body) const override
  {
    std::cout << "  [SMS] From: " << from
              << " | To: " << to << "\n"
              << "    Message: " << body << "\n";
    return true;
  }

  [[nodiscard]] std::string_view TransportName() const noexcept override
  {
    return "SMS";
  }
};

/// @brief Sends messages via Slack.
class SlackSender : public MessageSender
{
 public:
  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const override
  {
    std::cout << "  [Slack] Channel: " << to
              << " | From: " << from << "\n"
              << "    Subject: " << subject << "\n"
              << "    Body: " << body << "\n";
    return true;
  }

  [[nodiscard]] std::string_view TransportName() const noexcept override
  {
    return "Slack";
  }
};

// =============================================================================
// Abstraction Hierarchy (抽象层)
//
// This layer defines the high-level message logic. It holds a reference to a
// MessageSender (the "bridge") and delegates actual sending to it. Different
// message types (SimpleMessage, EncryptedMessage) refine the abstraction
// independently of the chosen transport.
// =============================================================================

/// @brief Base abstraction for a message.
class Message
{
 public:
  explicit Message(std::unique_ptr<MessageSender> sender)
      : sender_(std::move(sender))
  {
  }

  virtual ~Message() = default;

  /// Deliver the message. Delegates to the implementation layer.
  [[nodiscard]] bool Deliver(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const
  {
    std::cout << "Delivering via " << sender_->TransportName() << "...\n";
    return sender_->Send(from, to, subject, body);
  }

 protected:
  std::unique_ptr<MessageSender> sender_;
};

/// @brief A simple message with no extra processing.
class SimpleMessage : public Message
{
 public:
  using Message::Message;

  /// Send the message as-is.
  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const
  {
    return Deliver(from, to, subject, body);
  }
};

/// @brief A message that prepends a prefix to the subject and wraps the body.
class EncryptedMessage : public Message
{
 public:
  using Message::Message;

  /// Send with encryption decoration applied.
  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const
  {
    const std::string encrypted_subject =
        std::string("[ENCRYPTED] ").append(subject);
    const std::string encrypted_body =
        std::string("<<").append(body).append(">>");

    return Deliver(from, to, encrypted_subject, encrypted_body);
  }
};

// =============================================================================
// C++17 Variant-based Bridge (编译期多态桥接)
//
// An alternative implementation using std::variant instead of virtual
// dispatch. This demonstrates how the same Bridge pattern can be realized
// with compile-time polymorphism when the set of implementations is closed.
// =============================================================================

/// Closed set of sender types for the variant-based bridge.
using SenderVariant = std::variant<EmailSender, SmsSender, SlackSender>;

/// @brief A bridge that uses std::variant for compile-time polymorphism.
class VariantMessage
{
 public:
  explicit VariantMessage(SenderVariant sender) : sender_(std::move(sender))
  {
  }

  [[nodiscard]] bool Send(
      std::string_view from,
      std::string_view to,
      std::string_view subject,
      std::string_view body) const
  {
    return std::visit(
        [&](const auto& sender) -> bool
        {
          std::cout << "Delivering via " << sender.TransportName() << "...\n";
          return sender.Send(from, to, subject, body);
        },
        sender_);
  }

  [[nodiscard]] std::string_view TransportName() const noexcept
  {
    return std::visit(
        [](const auto& sender) -> std::string_view
        { return sender.TransportName(); },
        sender_);
  }

 private:
  SenderVariant sender_;
};

// =============================================================================
// Helper: Create a sender based on a channel name (demonstrates structured
// bindings with a factory function).
// =============================================================================

/// @brief Factory that maps a channel name to a concrete sender.
[[nodiscard]] std::unique_ptr<MessageSender> CreateSender(std::string_view channel)
{
  if (channel == "email")
  {
    return std::make_unique<EmailSender>();
  }
  if (channel == "sms")
  {
    return std::make_unique<SmsSender>();
  }
  if (channel == "slack")
  {
    return std::make_unique<SlackSender>();
  }
  return nullptr;
}

/// @brief Print a section separator for readable output.
void PrintSection(std::string_view title)
{
  std::cout << "\n========================================\n"
            << title << "\n"
            << "========================================\n";
}

// =============================================================================
// Main: Demonstrate the Bridge pattern
// =============================================================================

int main()
{
  constexpr std::string_view kFrom = "alice@example.com";
  constexpr std::string_view kTo = "bob@example.com";
  constexpr std::string_view kSubject = "Meeting";
  constexpr std::string_view kBody = "Let's meet at 3pm tomorrow.";

  // ---- 1. Virtual-dispatch bridge (runtime polymorphism) ----

  PrintSection("1. SimpleMessage + EmailSender (virtual bridge)");

  {
    auto email = std::make_unique<EmailSender>();
    SimpleMessage simple(std::move(email));
    (void)simple.Send(kFrom, kTo, kSubject, kBody);
  }

  PrintSection("2. SimpleMessage + SmsSender (virtual bridge)");

  {
    auto sms = std::make_unique<SmsSender>();
    SimpleMessage simple(std::move(sms));
    (void)simple.Send(kFrom, kTo, kSubject, kBody);
  }

  PrintSection("3. EncryptedMessage + SlackSender (virtual bridge)");

  {
    auto slack = std::make_unique<SlackSender>();
    EncryptedMessage encrypted(std::move(slack));
    (void)encrypted.Send(kFrom, "#engineering", kSubject, kBody);
  }

  // ---- 2. Factory + structured bindings ----

  PrintSection("4. Factory-created sender (structured binding demo)");

  {
    // Use structured binding to unpack a pair.
    auto [channel, target] = std::make_pair(std::string("sms"), std::string("bob"));
    if (auto sender = CreateSender(channel))
    {
      SimpleMessage msg(std::move(sender));
      (void)msg.Send(kFrom, target, kSubject, kBody);
    }
  }

  // ---- 3. Variant-based bridge (compile-time polymorphism) ----

  PrintSection("5. VariantMessage + EmailSender (variant bridge)");

  {
    VariantMessage msg(EmailSender{});
    (void)msg.Send(kFrom, kTo, kSubject, kBody);
  }

  PrintSection("6. VariantMessage + SmsSender (variant bridge)");

  {
    VariantMessage msg(SmsSender{});
    (void)msg.Send(kFrom, kTo, kSubject, kBody);
  }

  PrintSection("7. VariantMessage + SlackSender (variant bridge)");

  {
    VariantMessage msg(SlackSender{});
    (void)msg.Send(kFrom, "#general", kSubject, kBody);
  }

  // ---- 4. Demonstrate independent variation ----

  PrintSection("8. Same message type, different transports");

  {
    // EncryptedMessage can be paired with any transport.
    // This demonstrates the core value of the Bridge pattern:
    // abstraction and implementation vary independently.
    std::vector<std::pair<std::string, SenderVariant>> configs = {
        {"Email", EmailSender{}},
        {"SMS", SmsSender{}},
        {"Slack", SlackSender{}},
    };

    for (const auto& [name, sender] : configs)
    {
      std::cout << "  --- " << name << " ---\n";
      VariantMessage msg(sender);
      (void)msg.Send(kFrom, kTo, "Encrypted subject", "Encrypted body content");
    }
  }

  std::cout << "\nDone.\n";
  return 0;
}
