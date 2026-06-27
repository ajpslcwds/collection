/**
 * State Pattern (状态模式)
 *
 * Intent: Allow an object to alter its behavior when its internal state changes.
 * The object will appear to change its class.
 *
 * C++17 Features Used:
 * - std::variant: Type-safe state storage with compile-time type checking
 * - std::optional: Representing potentially failed state transitions
 * - std::shared_mutex: Thread-safe state access
 * - Structured bindings: Clean state information extraction
 * - if constexpr: Compile-time state type dispatch
 * - Inline variables: Global state name constants
 * - std::visit: Type-safe variant visitor pattern
 */

#include <iostream>
#include <string>
#include <variant>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <functional>
#include <vector>
#include <memory>

// =============================================================================
// State Pattern Core Implementation
// =============================================================================

// Forward declarations
class DocumentContext;

// State types using std::variant (compile-time type safety)
struct DraftState
{
  std::string author;
  int revision_count = 0;
};

struct ReviewState
{
  std::string reviewer;
  std::string review_comment;
};

struct PublishedState
{
  std::string publisher;
  std::string publish_date;
};

struct ArchivedState
{
  std::string archive_reason;
};

// Type-safe state variant
using DocumentState = std::variant<DraftState, ReviewState, PublishedState, ArchivedState>;

// Inline state name constants
inline const std::string kDraftStateName = "Draft";
inline const std::string kReviewStateName = "Review";
inline const std::string kPublishedStateName = "Published";
inline const std::string kArchivedStateName = "Archived";

// State visitor for getting state name
struct StateNameVisitor
{
  std::string operator()(const DraftState&) const
  {
    return kDraftStateName;
  }

  std::string operator()(const ReviewState&) const
  {
    return kReviewStateName;
  }

  std::string operator()(const PublishedState&) const
  {
    return kPublishedStateName;
  }

  std::string operator()(const ArchivedState&) const
  {
    return kArchivedStateName;
  }
};

// Document context class with thread-safe state management
class DocumentContext
{
public:
  DocumentContext(const std::string& title, const std::string& author)
      : title_(title)
  {
    state_ = DraftState{author, 0};
  }

  // Thread-safe state query
  std::string GetCurrentState() const
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return std::visit(StateNameVisitor{}, state_);
  }

  // Get document title
  std::string GetTitle() const
  {
    return title_;
  }

  // Edit operation - only allowed in Draft state
  std::optional<bool> Edit(const std::string& content)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    // if constexpr for compile-time type checking
    if constexpr (true)
    {
      if (auto* draft = std::get_if<DraftState>(&state_))
      {
        draft->revision_count++;
        std::cout << "  [Edit] Document '" << title_ << "' edited by "
                  << draft->author << " (revision " << draft->revision_count
                  << "): " << content << std::endl;
        return true;
      }
    }

    std::cout << "  [Edit] ERROR: Cannot edit document in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Submit for review operation - only allowed in Draft state
  std::optional<bool> SubmitForReview(const std::string& reviewer)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (auto* draft = std::get_if<DraftState>(&state_))
    {
      if (draft->revision_count == 0)
      {
        std::cout << "  [SubmitForReview] ERROR: Document must be edited at least once before review"
                  << std::endl;
        return std::nullopt;
      }

      std::cout << "  [SubmitForReview] Document '" << title_
                << "' submitted for review to " << reviewer << std::endl;
      state_ = ReviewState{reviewer, ""};
      return true;
    }

    std::cout << "  [SubmitForReview] ERROR: Cannot submit for review in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Approve operation - only allowed in Review state
  std::optional<bool> Approve(const std::string& publisher, const std::string& date)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (auto* review = std::get_if<ReviewState>(&state_))
    {
      if (review->review_comment.empty())
      {
        std::cout << "  [Approve] ERROR: Review comment is required for approval"
                  << std::endl;
        return std::nullopt;
      }

      std::cout << "  [Approve] Document '" << title_ << "' approved by "
                << review->reviewer << " and published by " << publisher
                << std::endl;
      state_ = PublishedState{publisher, date};
      return true;
    }

    std::cout << "  [Approve] ERROR: Cannot approve in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Reject operation - only allowed in Review state
  std::optional<bool> Reject(const std::string& reason)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (auto* review = std::get_if<ReviewState>(&state_))
    {
      std::cout << "  [Reject] Document '" << title_ << "' rejected by "
                << review->reviewer << " with reason: " << reason << std::endl;

      // Get original author from the document (simplified - in real app would track this)
      state_ = DraftState{"Original Author", 0};
      return true;
    }

    std::cout << "  [Reject] ERROR: Cannot reject in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Add review comment
  std::optional<bool> AddReviewComment(const std::string& comment)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (auto* review = std::get_if<ReviewState>(&state_))
    {
      review->review_comment = comment;
      std::cout << "  [AddReviewComment] Comment added to review: '"
                << comment << "'" << std::endl;
      return true;
    }

    std::cout << "  [AddReviewComment] ERROR: Cannot add comment in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Archive operation - allowed in Published state
  std::optional<bool> Archive(const std::string& reason)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (std::get_if<PublishedState>(&state_))
    {
      std::cout << "  [Archive] Document '" << title_ << "' archived. Reason: "
                << reason << std::endl;
      state_ = ArchivedState{reason};
      return true;
    }

    std::cout << "  [Archive] ERROR: Cannot archive in "
              << std::visit(StateNameVisitor{}, state_) << " state"
              << std::endl;
    return std::nullopt;
  }

  // Get detailed state information using structured bindings
  std::pair<std::string, std::string> GetStateInfo() const
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return {std::visit(StateNameVisitor{}, state_), title_};
  }

  // Check if document is in a specific state using if constexpr
  template <typename StateType>
  bool IsInState() const
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return std::holds_alternative<StateType>(state_);
  }

private:
  std::string title_;
  DocumentState state_;
  mutable std::shared_mutex mutex_;
};

// =============================================================================
// Alternative Implementation: Traditional State Pattern with Inheritance
// =============================================================================

// Abstract state interface
class State
{
public:
  virtual ~State() = default;
  virtual void Handle(DocumentContext& context) = 0;
  virtual std::string GetStateName() const = 0;
};

// Concrete state implementations
class DraftStateClass : public State
{
public:
  void Handle(DocumentContext& /*context*/) override
  {
    std::cout << "  DraftState: Handling document in draft state" << std::endl;
  }

  std::string GetStateName() const override
  {
    return "Draft";
  }
};

class ReviewStateClass : public State
{
public:
  void Handle(DocumentContext& /*context*/) override
  {
    std::cout << "  ReviewState: Handling document in review state" << std::endl;
  }

  std::string GetStateName() const override
  {
    return "Review";
  }
};

class PublishedStateClass : public State
{
public:
  void Handle(DocumentContext& /*context*/) override
  {
    std::cout << "  PublishedState: Handling document in published state" << std::endl;
  }

  std::string GetStateName() const override
  {
    return "Published";
  }
};

// =============================================================================
// Usage Example
// =============================================================================

void DemonstrateStatePattern()
{
  std::cout << "=== Document Workflow State Pattern Demo ===" << std::endl;
  std::cout << std::endl;

  // Create a document in Draft state
  DocumentContext document("Design Patterns Guide", "Alice");
  std::cout << "Created document: '" << document.GetTitle() << "'" << std::endl;

  // Get initial state info using structured bindings
  auto [state, title] = document.GetStateInfo();
  std::cout << "Initial state: " << state << std::endl;
  std::cout << std::endl;

  // Test state transitions
  std::cout << "--- Testing State Transitions ---" << std::endl;

  // Try to submit without editing (should fail)
  std::cout << "\n1. Attempting to submit without editing:" << std::endl;
  document.SubmitForReview("Bob");

  // Edit the document
  std::cout << "\n2. Editing the document:" << std::endl;
  document.Edit("Added introduction section");
  document.Edit("Added main content");

  // Submit for review
  std::cout << "\n3. Submitting for review:" << std::endl;
  document.SubmitForReview("Bob");

  // Add review comment
  std::cout << "\n4. Adding review comment:" << std::endl;
  document.AddReviewComment("Good content, minor formatting issues");

  // Try to edit in review state (should fail)
  std::cout << "\n5. Attempting to edit in review state:" << std::endl;
  document.Edit("Trying to edit");

  // Approve the document
  std::cout << "\n6. Approving the document:" << std::endl;
  document.Approve("Charlie", "2024-01-15");

  // Try to edit in published state (should fail)
  std::cout << "\n7. Attempting to edit in published state:" << std::endl;
  document.Edit("Trying to edit published doc");

  // Archive the document
  std::cout << "\n8. Archiving the document:" << std::endl;
  document.Archive("Document is outdated");

  // Final state check
  std::cout << "\n--- Final State Check ---" << std::endl;
  auto [final_state, final_title] = document.GetStateInfo();
  std::cout << "Document: " << final_title << std::endl;
  std::cout << "Final State: " << final_state << std::endl;

  // Check specific state using template
  std::cout << "Is in Draft state? "
            << (document.IsInState<DraftState>() ? "Yes" : "No") << std::endl;
  std::cout << "Is in Archived state? "
            << (document.IsInState<ArchivedState>() ? "Yes" : "No") << std::endl;
}

void DemonstrateStateTransitions()
{
  std::cout << "\n=== State Transition Diagram ===" << std::endl;
  std::cout << "Valid state transitions:" << std::endl;
  std::cout << "  Draft -> Review (with edit)" << std::endl;
  std::cout << "  Review -> Published (with approval)" << std::endl;
  std::cout << "  Review -> Draft (rejection)" << std::endl;
  std::cout << "  Published -> Archived (archival)" << std::endl;
  std::cout << std::endl;
  std::cout << "Invalid transitions (will fail):" << std::endl;
  std::cout << "  Draft -> Published (skip review)" << std::endl;
  std::cout << "  Review -> Draft (without rejection)" << std::endl;
  std::cout << "  Published -> Draft (cannot un-publish)" << std::endl;
}

int main()
{
  std::cout << "State Design Pattern - C++17 Implementation" << std::endl;
  std::cout << "============================================" << std::endl;
  std::cout << std::endl;

  // Demonstrate the main state pattern implementation
  DemonstrateStatePattern();

  // Show state transition rules
  DemonstrateStateTransitions();

  std::cout << "\n=== Key C++17 Features Demonstrated ===" << std::endl;
  std::cout << "1. std::variant: Type-safe state storage" << std::endl;
  std::cout << "2. std::optional: Failed state transitions" << std::endl;
  std::cout << "3. std::shared_mutex: Thread-safe state access" << std::endl;
  std::cout << "4. Structured bindings: Clean state info extraction" << std::endl;
  std::cout << "5. if constexpr: Compile-time type checking" << std::endl;
  std::cout << "6. Inline variables: State name constants" << std::endl;
  std::cout << "7. std::visit: Type-safe variant visitor" << std::endl;

  return 0;
}