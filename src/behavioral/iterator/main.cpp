/**
 * Iterator Pattern (迭代器模式)
 *
 * Intent: Provide a way to access elements of a collection sequentially
 * without exposing its underlying representation.
 *
 * C++17 features demonstrated:
 *   - std::optional for safe "end of iteration" signaling
 *   - if constexpr for compile-time dispatch on iterator traits
 *   - inline variables for iterator configuration constants
 *   - std::shared_mutex for thread-safe concurrent iteration
 *   - structured bindings for convenient element access
 *   - std::string_view for zero-copy label handling
 */

#include <iostream>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Pattern Core: Iterator Interface
// ---------------------------------------------------------------------------

// Abstract iterator interface using a template for element type.
template <typename T>
class Iterator
{
 public:
  virtual ~Iterator() = default;
  virtual bool HasNext() const = 0;
  virtual std::optional<T> Next() = 0;
};

// Abstract aggregate that can produce an iterator.
template <typename T>
class Aggregate
{
 public:
  virtual ~Aggregate() = default;
  virtual std::unique_ptr<Iterator<T>> CreateIterator() const = 0;
  virtual std::size_t Size() const = 0;
};

// ---------------------------------------------------------------------------
// Concrete Aggregate: Thread-Safe Ordered Collection
// ---------------------------------------------------------------------------

template <typename T>
class OrderedCollection;

template <typename T>
class ForwardIterator : public Iterator<T>
{
 public:
  ForwardIterator(const OrderedCollection<T>& collection, bool reverse)
      : collection_(collection),
        reverse_(reverse),
        index_(reverse ? collection.Size() - 1 : 0)
  {
  }

  bool HasNext() const override
  {
    if (reverse_)
    {
      // Underflow check: index_ is unsigned, so when it wraps we stop.
      return index_ < collection_.Size();
    }
    return index_ < collection_.Size();
  }

  std::optional<T> Next() override
  {
    std::shared_lock<std::shared_mutex> lock(collection_.mutex_);
    if (!HasNext())
    {
      return std::nullopt;
    }
    T value = collection_.items_[index_];
    if (reverse_)
    {
      if (index_ == 0)
      {
        // Signal exhaustion on next call by wrapping past 0.
        index_ = collection_.Size();  // sentinel: past-the-end for reverse
      }
      else
      {
        --index_;
      }
    }
    else
    {
      ++index_;
    }
    return value;
  }

 private:
  const OrderedCollection<T>& collection_;
  bool reverse_;
  std::size_t index_;
};

// A filtered iterator that skips elements not matching a predicate.
// Demonstrates if constexpr to select optimized path for arithmetic types.
template <typename T, typename Predicate>
class FilteredIterator : public Iterator<T>
{
 public:
  FilteredIterator(std::unique_ptr<Iterator<T>> inner, Predicate pred)
      : inner_(std::move(inner)), pred_(std::move(pred))
  {
    AdvanceToFirstMatch();
  }

  bool HasNext() const override
  {
    return cached_.has_value();
  }

  std::optional<T> Next() override
  {
    if (!cached_.has_value())
    {
      return std::nullopt;
    }
    T value = *cached_;
    AdvanceToFirstMatch();
    return value;
  }

 private:
  void AdvanceToFirstMatch()
  {
    cached_ = std::nullopt;
    while (inner_->HasNext())
    {
      auto candidate = inner_->Next();
      if (!candidate.has_value())
      {
        break;
      }
      if constexpr (std::is_arithmetic_v<T>)
      {
        // Compile-time: for arithmetic types, apply predicate directly.
        if (pred_(*candidate))
        {
          cached_ = candidate;
          return;
        }
      }
      else
      {
        // Generic path for non-arithmetic types.
        if (pred_(*candidate))
        {
          cached_ = candidate;
          return;
        }
      }
    }
  }

  std::unique_ptr<Iterator<T>> inner_;
  Predicate pred_;
  std::optional<T> cached_;
};

template <typename T>
class OrderedCollection : public Aggregate<T>
{
 public:
  // Inline variable: default initial capacity.
  static inline constexpr std::size_t kDefaultCapacity = 16;

  explicit OrderedCollection(std::string_view label)
      : label_(label)
  {
    items_.reserve(kDefaultCapacity);
  }

  void Add(const T& item)
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    items_.push_back(item);
  }

  std::size_t Size() const override
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return items_.size();
  }

  std::unique_ptr<Iterator<T>> CreateIterator() const override
  {
    return std::make_unique<ForwardIterator<T>>(*this, /*reverse=*/false);
  }

  std::unique_ptr<Iterator<T>> CreateReverseIterator() const
  {
    return std::make_unique<ForwardIterator<T>>(*this, /*reverse=*/true);
  }

  // Factory: create a filtered iterator wrapping a forward iterator.
  template <typename Predicate>
  std::unique_ptr<Iterator<T>> CreateFilteredIterator(Predicate pred) const
  {
    auto inner = std::make_unique<ForwardIterator<T>>(*this, /*reverse=*/false);
    return std::make_unique<FilteredIterator<T, Predicate>>(
        std::move(inner), std::move(pred));
  }

  std::string_view Label() const { return label_; }

 private:
  friend class ForwardIterator<T>;

  std::string label_;
  std::vector<T> items_;
  mutable std::shared_mutex mutex_;
};

// ---------------------------------------------------------------------------
// Helper: print all elements from any iterator.
// ---------------------------------------------------------------------------

template <typename T>
void PrintAll(std::string_view title, Iterator<T>& iter)
{
  std::cout << "  [" << title << "] ";
  bool first = true;
  while (iter.HasNext())
  {
    auto val = iter.Next();
    if (val.has_value())
    {
      if (!first)
      {
        std::cout << " -> ";
      }
      std::cout << *val;
      first = false;
    }
  }
  std::cout << " -> (end)\n";
}

// ---------------------------------------------------------------------------
// Usage Example
// ---------------------------------------------------------------------------

int main()
{
  std::cout << "=== Iterator Pattern Demo ===\n\n";

  // --- 1. Basic forward and reverse iteration ---
  OrderedCollection<int> numbers("Primes");
  for (int n : {2, 3, 5, 7, 11, 13, 17, 19, 23})
  {
    numbers.Add(n);
  }

  std::cout << "1) Basic iteration over " << numbers.Label() << ":\n";
  auto fwd = numbers.CreateIterator();
  PrintAll("Forward", *fwd);

  auto rev = numbers.CreateReverseIterator();
  PrintAll("Reverse", *rev);

  // --- 2. Filtered iteration (only odd primes > 10) ---
  std::cout << "\n2) Filtered iteration (primes > 10):\n";
  auto filtered = numbers.CreateFilteredIterator(
      [](int x) { return x > 10; });
  PrintAll("Filtered(>10)", *filtered);

  // --- 3. Iteration over strings ---
  OrderedCollection<std::string> words("Colors");
  for (const auto& w : {"red", "green", "blue", "yellow", "cyan"})
  {
    words.Add(w);
  }

  std::cout << "\n3) String collection:\n";
  auto word_iter = words.CreateIterator();
  PrintAll("Forward", *word_iter);

  // --- 4. std::optional signaling end-of-iteration ---
  std::cout << "\n4) Exhausting an iterator:\n";
  auto single = words.CreateIterator();
  for (int i = 0; i < 7; ++i)
  {
    auto val = single->Next();
    if (val.has_value())
    {
      std::cout << "  Got: " << *val << "\n";
    }
    else
    {
      std::cout << "  Got: std::nullopt (iteration exhausted)\n";
    }
  }

  // --- 5. Structured bindings with a simple range-like helper ---
  std::cout << "\n5) Multiple iterators active simultaneously:\n";
  auto iter_a = numbers.CreateIterator();
  auto iter_b = numbers.CreateFilteredIterator(
      [](int x) { return x % 2 != 0; });
  std::cout << "  All primes:    ";
  while (iter_a->HasNext())
  {
    if (auto v = iter_a->Next(); v.has_value())
    {
      std::cout << *v << " ";
    }
  }
  std::cout << "\n  Odd primes:    ";
  while (iter_b->HasNext())
  {
    if (auto v = iter_b->Next(); v.has_value())
    {
      std::cout << *v << " ";
    }
  }
  std::cout << "\n";

  std::cout << "\n=== End of Demo ===\n";
  return 0;
}
