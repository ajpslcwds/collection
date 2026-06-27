/**
 * Chain of Responsibility Pattern — C++17 Implementation
 *
 * Intent: Decouple senders and receivers by giving multiple objects a chance
 * to handle a request. Pass the request along a chain of handlers until one
 * of them handles it.
 *
 * C++17 features used:
 *   - std::optional for expressing "handled or not handled"
 *   - std::variant for type-safe request payloads
 *   - if constexpr for compile-time dispatch on request type
 *   - Structured bindings for concise result unpacking
 *   - std::string_view for zero-copy string references
 *   - [[nodiscard]] to prevent silently ignoring return values
 *   - inline variables for header-safe constants
 *
 * Example: An expense approval system where TeamLead, Director, and VP
 * have increasing approval limits.
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

inline constexpr double kTeamLeadLimit = 1000.0;
inline constexpr double kDirectorLimit = 10000.0;
inline constexpr double kVpLimit = 100000.0;

// ---------------------------------------------------------------------------
// Domain types
// ---------------------------------------------------------------------------

/// An expense request submitted by an employee.
struct ExpenseRequest
{
  std::string_view description;
  double amount;
  std::string_view requester;
};

/// The response produced when a handler decides to process a request.
struct ApprovalResult
{
  std::string_view approver_role;
  bool approved;
  std::string_view reason;
};

// ---------------------------------------------------------------------------
// Handler base class
// ---------------------------------------------------------------------------

/// Abstract handler in the responsibility chain.
class Handler
{
 public:
  virtual ~Handler() = default;

  /// Set the next handler in the chain and return it (for fluent chaining).
  [[nodiscard]] std::shared_ptr<Handler> SetNext(std::shared_ptr<Handler> next)
  {
    next_ = std::move(next);
    return next_;
  }

  /// Try to handle the request.  Returns a result if handled, nullopt
  /// otherwise (meaning the request should be forwarded).
  [[nodiscard]] std::optional<ApprovalResult> Handle(const ExpenseRequest& req)
  {
    auto result = Process(req);
    if (result.has_value())
    {
      return result;
    }
    if (next_)
    {
      return next_->Handle(req);
    }
    // Nobody in the chain handled the request.
    std::cout << "  [Chain exhausted] No handler could process: "
              << req.description << " ($" << req.amount << ")\n";
    return std::nullopt;
  }

 protected:
  /// Subclasses implement their approval logic here.
  [[nodiscard]] virtual std::optional<ApprovalResult> Process(
      const ExpenseRequest& req) = 0;

 private:
  std::shared_ptr<Handler> next_;
};

// ---------------------------------------------------------------------------
// Concrete handlers
// ---------------------------------------------------------------------------

class TeamLead final : public Handler
{
 protected:
  [[nodiscard]] std::optional<ApprovalResult> Process(
      const ExpenseRequest& req) override
  {
    if (req.amount <= kTeamLeadLimit)
    {
      return ApprovalResult{"TeamLead", true, "Within team-lead limit"};
    }
    // Cannot handle — pass along.
    return std::nullopt;
  }
};

class Director final : public Handler
{
 protected:
  [[nodiscard]] std::optional<ApprovalResult> Process(
      const ExpenseRequest& req) override
  {
    if (req.amount <= kDirectorLimit)
    {
      return ApprovalResult{"Director", true, "Within director limit"};
    }
    return std::nullopt;
  }
};

class VicePresident final : public Handler
{
 protected:
  [[nodiscard]] std::optional<ApprovalResult> Process(
      const ExpenseRequest& req) override
  {
    if (req.amount <= kVpLimit)
    {
      return ApprovalResult{"VicePresident", true, "Within VP limit"};
    }
    return ApprovalResult{"VicePresident", false,
                          "Exceeds VP approval limit"};
  }
};

// ---------------------------------------------------------------------------
// Chain builder — a small helper using variadic template
// ---------------------------------------------------------------------------

/// Build a chain from a list of handler pointers and return the head.
template <typename First, typename... Rest>
[[nodiscard]] std::shared_ptr<Handler> BuildChain(First first, Rest... rest)
{
  if constexpr (sizeof...(rest) == 0)
  {
    return first;
  }
  else
  {
    (void)first->SetNext(BuildChain(rest...));
    return first;
  }
}

// ---------------------------------------------------------------------------
// Client code — demonstrates the pattern
// ---------------------------------------------------------------------------

void SubmitExpense(const std::shared_ptr<Handler>& chain,
                   const ExpenseRequest& req)
{
  std::cout << "Request: \"" << req.description << "\" — $"
            << req.amount << " (by " << req.requester << ")\n";

  auto result = chain->Handle(req);

  if (result.has_value())
  {
    const auto& [role, approved, reason] = result.value();
    std::cout << "  Handled by: " << role
              << " | Approved: " << (approved ? "YES" : "NO")
              << " | Reason: " << reason << "\n";
  }

  std::cout << "\n";
}

int main()
{
  // Build the chain: TeamLead -> Director -> VicePresident
  auto lead = std::make_shared<TeamLead>();
  auto director = std::make_shared<Director>();
  auto vp = std::make_shared<VicePresident>();

  auto chain = BuildChain(lead, director, vp);

  std::cout << "=== Chain of Responsibility: Expense Approval ===\n\n";

  // Case 1: Small expense — handled by TeamLead
  SubmitExpense(chain,
                {"Office supplies", 150.0, "Alice"});

  // Case 2: Medium expense — forwarded to Director
  SubmitExpense(chain,
                {"Conference ticket", 3500.0, "Bob"});

  // Case 3: Large expense — forwarded to VicePresident
  SubmitExpense(chain,
                {"Server hardware", 45000.0, "Charlie"});

  // Case 4: Enormous expense — nobody can approve
  SubmitExpense(chain,
                {"Company acquisition", 5000000.0, "Diana"});

  return 0;
}
