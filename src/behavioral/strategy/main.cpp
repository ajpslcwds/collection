/**
 * Strategy Pattern — C++17 Implementation
 *
 * Intent: Define a family of algorithms, encapsulate each one, and make them
 * interchangeable at runtime. This example demonstrates two scenarios:
 *   1. Sorting strategies (BubbleSort, QuickSort, MergeSort)
 *   2. Payment strategies (CreditCard, Alipay, WeChatPay)
 *
 * C++17 features used:
 *   - std::variant / std::visit for type-safe polymorphic dispatch
 *   - std::optional for nullable strategy holding
 *   - std::function for lightweight callable strategies
 *   - Structured bindings for clean result unpacking
 *   - if constexpr for compile-time branching
 *   - Fold expressions for batch strategy aggregation
 *   - Class Template Argument Deduction (CTAD)
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// ============================================================================
// Scenario 1: Sorting Strategies using std::variant + std::visit
// ============================================================================

namespace sorting {

// --- Strategy interfaces (tag types for variant) ---

struct BubbleSort
{
  static constexpr const char* kName = "BubbleSort";
};

struct QuickSort
{
  static constexpr const char* kName = "QuickSort";
};

struct MergeSort
{
  static constexpr const char* kName = "MergeSort";
};

// --- Concrete algorithm implementations ---

void DoBubbleSort(std::vector<int>& data)
{
  const int n = static_cast<int>(data.size());
  for (int i = 0; i < n - 1; ++i)
  {
    for (int j = 0; j < n - i - 1; ++j)
    {
      if (data[j] > data[j + 1])
      {
        std::swap(data[j], data[j + 1]);
      }
    }
  }
}

void DoQuickSort(std::vector<int>& data, int low, int high)
{
  if (low >= high)
  {
    return;
  }
  int pivot = data[high];
  int i = low - 1;
  for (int j = low; j < high; ++j)
  {
    if (data[j] < pivot)
    {
      std::swap(data[++i], data[j]);
    }
  }
  std::swap(data[i + 1], data[high]);
  int pi = i + 1;
  DoQuickSort(data, low, pi - 1);
  DoQuickSort(data, pi + 1, high);
}

void DoMergeSort(std::vector<int>& data)
{
  if (data.size() <= 1)
  {
    return;
  }
  int mid = static_cast<int>(data.size()) / 2;
  std::vector<int> left(data.begin(), data.begin() + mid);
  std::vector<int> right(data.begin() + mid, data.end());
  DoMergeSort(left);
  DoMergeSort(right);
  data.clear();
  std::merge(left.begin(), left.end(), right.begin(), right.end(),
             std::back_inserter(data));
}

// --- Strategy variant type ---

using SortStrategy = std::variant<BubbleSort, QuickSort, MergeSort>;

// --- Visitor that executes the chosen strategy ---

struct SortVisitor
{
  std::vector<int>& data;

  void operator()(BubbleSort /*strategy*/) const
  {
    DoBubbleSort(data);
  }

  void operator()(QuickSort /*strategy*/) const
  {
    DoQuickSort(data, 0, static_cast<int>(data.size()) - 1);
  }

  void operator()(MergeSort /*strategy*/) const
  {
    DoMergeSort(data);
  }
};

// --- Context that holds and executes a strategy ---

class Sorter
{
 public:
  explicit Sorter(SortStrategy strategy)
      : strategy_(std::move(strategy))
  {
  }

  void SetStrategy(SortStrategy strategy)
  {
    strategy_ = std::move(strategy);
  }

  // Returns the name of the current strategy via a visitor.
  std::string Name() const
  {
    return std::visit(
        [](const auto& s) -> std::string { return s.kName; }, strategy_);
  }

  // Executes the current strategy on the given data.
  void Sort(std::vector<int>& data) const
  {
    std::visit(SortVisitor{data}, strategy_);
  }

 private:
  SortStrategy strategy_;
};

}  // namespace sorting

// ============================================================================
// Scenario 2: Payment Strategies using std::variant + structured bindings
// ============================================================================

namespace payment {

// --- Payment result (returned by all strategies) ---

struct PaymentResult
{
  bool success;
  std::string transaction_id;
  std::string message;
};

// --- Strategy types ---

struct CreditCard
{
  std::string card_number;
  std::string holder_name;
};

struct Alipay
{
  std::string account;
};

struct WeChatPay
{
  std::string open_id;
};

// --- Process payment for each strategy type ---

PaymentResult ProcessPayment(const CreditCard& card, double amount)
{
  return {
      true,
      "CC-" + card.card_number.substr(card.card_number.size() - 4),
      "Charged $" + std::to_string(amount) + " to credit card of " +
          card.holder_name,
  };
}

PaymentResult ProcessPayment(const Alipay& ali, double amount)
{
  return {
      true,
      "ALI-" + ali.account,
      "Charged $" + std::to_string(amount) + " via Alipay (" + ali.account +
          ")",
  };
}

PaymentResult ProcessPayment(const WeChatPay& wx, double amount)
{
  return {
      true,
      "WX-" + wx.open_id.substr(0, 6),
      "Charged $" + std::to_string(amount) + " via WeChatPay (" + wx.open_id +
          ")",
  };
}

// --- Strategy variant ---

using PayStrategy = std::variant<CreditCard, Alipay, WeChatPay>;

// --- Context ---

class PaymentProcessor
{
 public:
  explicit PaymentProcessor(PayStrategy strategy)
      : strategy_(std::move(strategy))
  {
  }

  void SetStrategy(PayStrategy strategy)
  {
    strategy_ = std::move(strategy);
  }

  // Execute payment using the current strategy.
  // Structured bindings are used by the caller to unpack the result.
  PaymentResult Pay(double amount) const
  {
    return std::visit(
        [amount](const auto& strategy) { return ProcessPayment(strategy, amount); },
        strategy_);
  }

 private:
  PayStrategy strategy_;
};

}  // namespace payment

// ============================================================================
// Scenario 3: Lightweight strategies using std::function + std::optional
// ============================================================================

namespace discount {

// A discount strategy is simply a function: original price -> discounted price.
using DiscountFunc = std::function<double(double)>;

// Concrete discount strategies as free functions / lambdas.
double NoDiscount(double price)
{
  return price;
}

auto PercentageOff(double percent)
{
  return [percent](double price) { return price * (1.0 - percent / 100.0); };
}

auto FlatDiscount(double amount)
{
  return [amount](double price) { return std::max(0.0, price - amount); };
}

// --- Context using std::optional<DiscountFunc> ---

class ShoppingCart
{
 public:
  void SetDiscount(std::optional<DiscountFunc> discount)
  {
    discount_ = std::move(discount);
  }

  void AddItem(const std::string& name, double price)
  {
    items_.push_back({name, price});
  }

  // Returns {subtotal, total_after_discount} using structured bindings.
  std::pair<double, double> Checkout() const
  {
    double subtotal = 0.0;
    for (const auto& [name, price] : items_)
    {
      (void)name;
      subtotal += price;
    }
    double total = discount_.has_value() ? (*discount_)(subtotal) : subtotal;
    return {subtotal, total};
  }

 private:
  struct Item
  {
    std::string name;
    double price;
  };

  std::vector<Item> items_;
  std::optional<DiscountFunc> discount_;
};

}  // namespace discount

// ============================================================================
// Scenario 4: Fold expression for batch strategy execution
// ============================================================================

namespace batch {

// Execute multiple strategies on data and return all results.
// Uses a fold expression to apply each strategy in a parameter pack.
template <typename... Strategies>
std::vector<std::vector<int>> ApplyAll(std::vector<int> data,
                                       Strategies... strategies)
{
  std::vector<std::vector<int>> results;
  // Fold expression: apply each strategy to a fresh copy of data.
  (results.push_back([&]() {
    auto copy = data;
    strategies(copy);
    return copy;
  }()),
   ...);
  return results;
}

// Simple sort functions for the batch demo.
void Ascending(std::vector<int>& v)
{
  std::sort(v.begin(), v.end());
}

void Descending(std::vector<int>& v)
{
  std::sort(v.begin(), v.end(), std::greater<>{});
}

void RotateLeft(std::vector<int>& v)
{
  if (!v.empty())
  {
    std::rotate(v.begin(), v.begin() + 1, v.end());
  }
}

}  // namespace batch

// ============================================================================
// Demo helpers
// ============================================================================

void PrintVector(const std::string& label, const std::vector<int>& v)
{
  std::cout << label << ": [";
  for (size_t i = 0; i < v.size(); ++i)
  {
    if (i > 0)
    {
      std::cout << ", ";
    }
    std::cout << v[i];
  }
  std::cout << "]\n";
}

// ============================================================================
// Main
// ============================================================================

int main()
{
  std::cout << "=== Strategy Pattern — C++17 Demo ===\n\n";

  // --- Scenario 1: Sorting strategies via std::variant ---
  std::cout << "--- Sorting Strategies (std::variant + std::visit) ---\n";

  std::vector<int> data = {38, 27, 43, 3, 9, 82, 10};

  sorting::Sorter sorter{sorting::BubbleSort{}};
  auto bubble_data = data;
  PrintVector("Original", bubble_data);
  sorter.Sort(bubble_data);
  PrintVector(sorter.Name(), bubble_data);

  sorter.SetStrategy(sorting::QuickSort{});
  auto quick_data = data;
  sorter.Sort(quick_data);
  PrintVector(sorter.Name(), quick_data);

  sorter.SetStrategy(sorting::MergeSort{});
  auto merge_data = data;
  sorter.Sort(merge_data);
  PrintVector(sorter.Name(), merge_data);

  // --- Scenario 2: Payment strategies with structured bindings ---
  std::cout << "\n--- Payment Strategies (std::variant + structured bindings) ---\n";

  payment::PaymentProcessor processor{payment::CreditCard{"1234567890123456",
                                                           "Alice"}};
  auto [ok1, txn1, msg1] = processor.Pay(99.99);
  std::cout << "[" << (ok1 ? "OK" : "FAIL") << "] " << msg1
            << "  txn=" << txn1 << "\n";

  processor.SetStrategy(payment::Alipay{"alice@example.com"});
  auto [ok2, txn2, msg2] = processor.Pay(49.50);
  std::cout << "[" << (ok2 ? "OK" : "FAIL") << "] " << msg2
            << "  txn=" << txn2 << "\n";

  processor.SetStrategy(payment::WeChatPay{"wxid_abc123def456"});
  auto [ok3, txn3, msg3] = processor.Pay(128.00);
  std::cout << "[" << (ok3 ? "OK" : "FAIL") << "] " << msg3
            << "  txn=" << txn3 << "\n";

  // --- Scenario 3: Discount strategies via std::function + std::optional ---
  std::cout << "\n--- Discount Strategies (std::function + std::optional) ---\n";

  discount::ShoppingCart cart;
  cart.AddItem("C++ Primer", 59.99);
  cart.AddItem("Effective Modern C++", 45.99);

  // No discount
  auto [sub1, total1] = cart.Checkout();
  std::cout << "No discount:      subtotal=$" << sub1 << "  total=$" << total1
            << "\n";

  // 20% off
  cart.SetDiscount(discount::PercentageOff(20.0));
  auto [sub2, total2] = cart.Checkout();
  std::cout << "20% off:          subtotal=$" << sub2 << "  total=$" << total2
            << "\n";

  // Flat $15 off
  cart.SetDiscount(discount::FlatDiscount(15.0));
  auto [sub3, total3] = cart.Checkout();
  std::cout << "$15 off:          subtotal=$" << sub3 << "  total=$" << total3
            << "\n";

  // Remove discount (std::optional -> nullopt)
  cart.SetDiscount(std::nullopt);
  auto [sub4, total4] = cart.Checkout();
  std::cout << "Discount removed: subtotal=$" << sub4 << "  total=$" << total4
            << "\n";

  // --- Scenario 4: Fold expression for batch strategy application ---
  std::cout << "\n--- Batch Strategies (fold expression) ---\n";

  std::vector<int> batch_data = {5, 3, 8, 1, 9, 2};
  PrintVector("Original", batch_data);

  auto results = batch::ApplyAll(batch_data, batch::Ascending,
                                 batch::Descending, batch::RotateLeft);

  PrintVector("Ascending", results[0]);
  PrintVector("Descending", results[1]);
  PrintVector("RotateLeft", results[2]);

  std::cout << "\nDone.\n";
  return 0;
}
