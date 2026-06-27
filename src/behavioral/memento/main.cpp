/**
 * Memento Pattern -- C++17 Implementation
 *
 * Intent: Without violating encapsulation, capture and externalize an object's
 * internal state so that the object can be restored to this state later.
 *
 * C++17 features used:
 *   - std::shared_ptr for memento ownership (shared between undo/redo stacks)
 *   - std::optional for nullable selection ranges and fallible undo/redo
 *   - std::variant for type-safe editor operations (insert, delete, move)
 *   - std::string_view for zero-copy read-only access to memento data
 *   - [[nodiscard]] to prevent ignoring important return values
 *   - inline constexpr for in-class constants (ODR-safe)
 *   - Structured bindings for concise unpacking of selection ranges
 *
 * Example: A text editor with full undo/redo, selection tracking, and
 * checkpoint-based history management.
 */

#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

// ---------------------------------------------------------------------------
// Memento -- stores a snapshot of the editor's internal state
// ---------------------------------------------------------------------------

/// Immutable snapshot of the editor's internal state.
/// The caretaker (HistoryManager) stores these but never inspects their
/// contents; only the Originator (TextEditor) reads them back.
class EditorMemento
{
 public:
  EditorMemento(std::string content,
                std::size_t cursor_pos,
                std::optional<std::pair<std::size_t, std::size_t>> selection,
                std::string timestamp)
      : content_(std::move(content)),
        cursor_pos_(cursor_pos),
        selection_(selection),
        timestamp_(std::move(timestamp))
  {
  }

  /// Read-only accessors -- the caretaker should not need these.
  [[nodiscard]] std::string_view GetContent() const { return content_; }
  [[nodiscard]] std::size_t GetCursor() const { return cursor_pos_; }

  [[nodiscard]] std::optional<std::pair<std::size_t, std::size_t>>
  GetSelection() const
  {
    return selection_;
  }

  [[nodiscard]] std::string_view GetTimestamp() const { return timestamp_; }

 private:
  std::string content_;
  std::size_t cursor_pos_;
  std::optional<std::pair<std::size_t, std::size_t>> selection_;
  std::string timestamp_;
};

// ---------------------------------------------------------------------------
// HistoryManager (Caretaker) -- stores mementos, manages undo/redo stacks
// ---------------------------------------------------------------------------

/// Manages undo/redo stacks of editor mementos.
/// This is the "Caretaker" in the Memento pattern: it stores snapshots
/// but never examines or modifies their contents.
class HistoryManager
{
 public:
  static inline constexpr std::size_t k_default_max_history = 100;

  explicit HistoryManager(std::size_t max_history = k_default_max_history)
      : max_history_(max_history)
  {
  }

  /// Save a new state snapshot. Discards any existing redo history.
  void SaveState(std::shared_ptr<EditorMemento> memento)
  {
    redo_stack_.clear();
    undo_stack_.push_back(std::move(memento));

    if (undo_stack_.size() > max_history_)
    {
      undo_stack_.erase(undo_stack_.begin());
    }
  }

  /// Undo: pop the top of the undo stack and return it for restoration.
  /// The caller should push its current state onto the redo stack first
  /// by calling PushRedo() before calling this.
  [[nodiscard]] std::optional<std::shared_ptr<EditorMemento>> PopUndo()
  {
    if (undo_stack_.empty())
    {
      return std::nullopt;
    }

    auto memento = std::move(undo_stack_.back());
    undo_stack_.pop_back();
    return memento;
  }

  /// Redo: pop the top of the redo stack and return it for restoration.
  /// The caller should push its current state onto the undo stack first
  /// by calling PushUndo() before calling this.
  [[nodiscard]] std::optional<std::shared_ptr<EditorMemento>> PopRedo()
  {
    if (redo_stack_.empty())
    {
      return std::nullopt;
    }

    auto memento = std::move(redo_stack_.back());
    redo_stack_.pop_back();
    return memento;
  }

  /// Push a state onto the redo stack (used during undo).
  void PushRedo(std::shared_ptr<EditorMemento> memento)
  {
    redo_stack_.push_back(std::move(memento));
  }

  /// Push a state onto the undo stack (used during redo).
  void PushUndo(std::shared_ptr<EditorMemento> memento)
  {
    undo_stack_.push_back(std::move(memento));
  }

  [[nodiscard]] bool CanUndo() const { return !undo_stack_.empty(); }
  [[nodiscard]] bool CanRedo() const { return !redo_stack_.empty(); }

  [[nodiscard]] std::size_t GetUndoSize() const
  {
    return undo_stack_.size();
  }

  [[nodiscard]] std::size_t GetRedoSize() const
  {
    return redo_stack_.size();
  }

  void Clear()
  {
    undo_stack_.clear();
    redo_stack_.clear();
  }

 private:
  std::vector<std::shared_ptr<EditorMemento>> undo_stack_;
  std::vector<std::shared_ptr<EditorMemento>> redo_stack_;
  std::size_t max_history_;
};

// ---------------------------------------------------------------------------
// Editor operations -- modeled as a variant for type safety
// ---------------------------------------------------------------------------

/// Insert text at the cursor position.
struct InsertOp
{
  std::string text;
};

/// Delete `count` characters starting at `position`.
struct DeleteOp
{
  std::size_t position;
  std::size_t count;
};

/// Move the cursor to a new position.
struct MoveCursorOp
{
  std::size_t new_position;
};

/// Select a range of text.
struct SelectOp
{
  std::size_t begin;
  std::size_t end;
};

/// Type-safe editor operation.
using EditorOp = std::variant<InsertOp, DeleteOp, MoveCursorOp, SelectOp>;

// ---------------------------------------------------------------------------
// TextEditor (Originator) -- the object whose state we want to save/restore
// ---------------------------------------------------------------------------

/// A simple text editor that supports undo/redo via the Memento pattern.
/// This is the "Originator": it creates mementos to save its state and
/// restores from mementos to roll back.
class TextEditor
{
 public:
  explicit TextEditor(std::string filename)
      : filename_(std::move(filename))
  {
  }

  // --- State modification operations ---

  /// Insert text at the current cursor position.
  void Insert(std::string_view text)
  {
    SaveCheckpoint();
    content_.insert(cursor_pos_, text);
    cursor_pos_ += text.size();
    std::cout << "  [Editor] Inserted \"" << text << "\" at position "
              << (cursor_pos_ - text.size()) << "\n";
  }

  /// Delete `count` characters starting at `position`.
  void Delete(std::size_t position, std::size_t count)
  {
    if (position >= content_.size())
    {
      std::cout << "  [Editor] Delete: position " << position
                << " out of range.\n";
      return;
    }

    SaveCheckpoint();

    std::size_t actual_count = std::min(count, content_.size() - position);
    std::string deleted = content_.substr(position, actual_count);
    content_.erase(position, actual_count);

    if (cursor_pos_ > position + actual_count)
    {
      cursor_pos_ -= actual_count;
    }
    else if (cursor_pos_ > position)
    {
      cursor_pos_ = position;
    }

    if (selection_.has_value())
    {
      selection_ = std::nullopt;
    }

    std::cout << "  [Editor] Deleted \"" << deleted << "\" from position "
              << position << "\n";
  }

  /// Move the cursor to a new position.
  void MoveCursor(std::size_t new_position)
  {
    SaveCheckpoint();
    cursor_pos_ = std::min(new_position, content_.size());
    std::cout << "  [Editor] Cursor moved to " << cursor_pos_ << "\n";
  }

  /// Select a range of text.
  void Select(std::size_t begin, std::size_t end)
  {
    SaveCheckpoint();
    selection_ = std::make_pair(begin, end);
    std::cout << "  [Editor] Selected range [" << begin << ", " << end
              << ")\n";
  }

  /// Clear the current selection.
  void ClearSelection()
  {
    selection_ = std::nullopt;
  }

  // --- Undo / Redo ---

  /// Undo the last operation. Returns true if successful.
  [[nodiscard]] bool Undo()
  {
    // Save current state to redo stack, then restore from undo stack top.
    auto current = CreateMemento();

    auto result = history_.PopUndo();
    if (!result.has_value())
    {
      std::cout << "  [Editor] Nothing to undo.\n";
      return false;
    }

    // Push current state to redo so we can redo later.
    history_.PushRedo(std::move(current));

    // Restore from the memento we popped.
    RestoreFromMemento(**result);
    std::cout << "  [Editor] Undo: restored to \""
              << (*result)->GetTimestamp() << "\"\n";
    return true;
  }

  /// Redo the last undone operation. Returns true if successful.
  [[nodiscard]] bool Redo()
  {
    // Save current state to undo stack, then restore from redo stack top.
    auto current = CreateMemento();

    auto result = history_.PopRedo();
    if (!result.has_value())
    {
      std::cout << "  [Editor] Nothing to redo.\n";
      return false;
    }

    // Push current state to undo so we can undo later.
    history_.PushUndo(std::move(current));

    // Restore from the memento we popped.
    RestoreFromMemento(**result);
    std::cout << "  [Editor] Redo: restored to \""
              << (*result)->GetTimestamp() << "\"\n";
    return true;
  }

  // --- Accessors ---

  [[nodiscard]] std::string_view GetContent() const { return content_; }

  [[nodiscard]] std::size_t GetCursor() const { return cursor_pos_; }

  [[nodiscard]] std::optional<std::pair<std::size_t, std::size_t>>
  GetSelection() const
  {
    return selection_;
  }

  [[nodiscard]] std::string_view GetFilename() const { return filename_; }

  [[nodiscard]] bool CanUndo() const { return history_.CanUndo(); }
  [[nodiscard]] bool CanRedo() const { return history_.CanRedo(); }

  /// Display the current editor state.
  void Display() const
  {
    std::cout << "  +-- " << filename_ << " ----------------------\n";
    std::cout << "  | Content: \"" << content_ << "\"\n";
    std::cout << "  | Cursor:  " << cursor_pos_;

    if (selection_.has_value())
    {
      auto [begin, end] = *selection_;
      std::cout << "  Selection: [" << begin << ", " << end << ")";
    }
    std::cout << "\n";

    std::cout << "  | History: " << history_.GetUndoSize() << " undo, "
              << history_.GetRedoSize() << " redo\n";
    std::cout << "  +--------------------------------------\n";
  }

 private:
  /// Save the current state as a checkpoint before an operation.
  void SaveCheckpoint()
  {
    auto memento = CreateMemento();
    history_.SaveState(std::move(memento));
  }

  /// Create a memento from the current state (Originator creates memento).
  [[nodiscard]] std::shared_ptr<EditorMemento> CreateMemento()
  {
    static std::size_t op_counter = 0;
    std::string timestamp = "op#" + std::to_string(++op_counter);

    return std::make_shared<EditorMemento>(
        content_, cursor_pos_, selection_, std::move(timestamp));
  }

  /// Restore the editor state from a memento.
  void RestoreFromMemento(const EditorMemento& memento)
  {
    content_ = std::string{memento.GetContent()};
    cursor_pos_ = memento.GetCursor();
    selection_ = memento.GetSelection();
  }

  std::string content_;
  std::size_t cursor_pos_ = 0;
  std::optional<std::pair<std::size_t, std::size_t>> selection_;
  std::string filename_;
  HistoryManager history_;
};

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

void PrintSection(std::string_view title)
{
  std::cout << "\n=== " << title << " ===\n";
}

int main()
{
  TextEditor editor{"document.txt"};

  std::cout << "=== Memento Pattern: Text Editor with Undo/Redo ===\n";

  // --- Initial state ---
  PrintSection("Initial state");
  editor.Display();

  // --- Insert some text ---
  PrintSection("Insert text");
  editor.Insert("Hello, World!");
  editor.Display();

  // --- Move cursor and insert more ---
  PrintSection("Move cursor and insert");
  editor.MoveCursor(7);
  editor.Insert("Beautiful ");
  editor.Display();

  // --- Undo the last insert ---
  PrintSection("Undo last insert");
  (void)editor.Undo();
  editor.Display();

  // --- Undo again (back to initial insert) ---
  PrintSection("Undo again");
  (void)editor.Undo();
  editor.Display();

  // --- Redo ---
  PrintSection("Redo");
  (void)editor.Redo();
  editor.Display();

  // --- Delete some text ---
  PrintSection("Delete text");
  editor.Delete(0, 6);
  editor.Display();

  // --- Select a range ---
  PrintSection("Select range");
  editor.Select(0, 5);
  editor.Display();

  // --- Structured binding to inspect selection ---
  if (auto sel = editor.GetSelection(); sel.has_value())
  {
    auto [begin, end] = *sel;
    std::cout << "  [Inspect] Selection range: [" << begin << ", "
              << end << ")\n";
  }

  // --- Undo selection ---
  PrintSection("Undo selection");
  (void)editor.Undo();
  editor.Display();

  // --- Undo delete ---
  PrintSection("Undo delete");
  (void)editor.Undo();
  editor.Display();

  // --- Build up more history ---
  PrintSection("Multiple operations");
  editor.Insert("AAA");
  editor.Insert("BBB");
  editor.Insert("CCC");
  editor.Display();

  // --- Undo all three ---
  PrintSection("Undo three inserts");
  (void)editor.Undo();
  (void)editor.Undo();
  (void)editor.Undo();
  editor.Display();

  // --- Redo all three ---
  PrintSection("Redo three inserts");
  (void)editor.Redo();
  (void)editor.Redo();
  (void)editor.Redo();
  editor.Display();

  // --- Demonstrate variant-based operation dispatch ---
  PrintSection("Variant-based operation dispatch");
  std::vector<EditorOp> operations;
  operations.emplace_back(InsertOp{"[from variant] "});
  operations.emplace_back(MoveCursorOp{0});
  operations.emplace_back(SelectOp{0, 14});

  for (const auto& op : operations)
  {
    std::visit([&editor](const auto& operation)
    {
      using T = std::decay_t<decltype(operation)>;
      if constexpr (std::is_same_v<T, InsertOp>)
      {
        std::cout << "  [Visitor] Insert: \"" << operation.text << "\"\n";
        editor.Insert(operation.text);
      }
      else if constexpr (std::is_same_v<T, DeleteOp>)
      {
        std::cout << "  [Visitor] Delete: pos=" << operation.position
                  << " count=" << operation.count << "\n";
        editor.Delete(operation.position, operation.count);
      }
      else if constexpr (std::is_same_v<T, MoveCursorOp>)
      {
        std::cout << "  [Visitor] MoveCursor: " << operation.new_position
                  << "\n";
        editor.MoveCursor(operation.new_position);
      }
      else if constexpr (std::is_same_v<T, SelectOp>)
      {
        std::cout << "  [Visitor] Select: [" << operation.begin << ", "
                  << operation.end << ")\n";
        editor.Select(operation.begin, operation.end);
      }
    }, op);
  }
  editor.Display();

  // --- Final summary ---
  PrintSection("Final summary");
  std::cout << "  Document content: \"" << editor.GetContent() << "\"\n";
  std::cout << "  Cursor position:  " << editor.GetCursor() << "\n";
  std::cout << "  Can undo: " << std::boolalpha << editor.CanUndo() << "\n";
  std::cout << "  Can redo: " << editor.CanRedo() << "\n";

  return 0;
}
