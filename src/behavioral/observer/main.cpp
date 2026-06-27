/**
 * Observer Pattern (观察者模式) — Stock Market Price Tracking System
 *
 * Intent: Define a one-to-many dependency so that when one object (Subject)
 * changes state, all dependents (Observers) are notified and updated
 * automatically.
 *
 * C++17 features used:
 *   - std::variant           : type-safe event union replacing class hierarchies
 *   - std::visit + Overloaded: compile-time dispatch on variant alternatives
 *   - std::optional          : represent "no previous price" without sentinel values
 *   - std::shared_mutex      : reader-writer lock for concurrent observer access
 *   - std::string_view       : zero-copy read-only string parameters
 *   - inline variables       : module-level constants with ODR safety
 *   - Class Template Argument Deduction (CTAD) for Overloaded helper
 *
 * Compile: g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread main.cpp -o observer
 */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

// ============================================================================
// Section 1: Inline Constants (C++17 inline variables)
// ============================================================================

namespace observer {

inline constexpr double kDefaultPrice = 0.0;
inline constexpr int kMinVolumeThreshold = 1;

// ============================================================================
// Section 2: Event Types (std::variant replaces event class hierarchy)
// ============================================================================

// A price-change event carries the symbol, old price, and new price.
struct PriceChanged {
  std::string symbol;
  double old_price;
  double new_price;
};

// A volume-alert event fires when trading volume exceeds a threshold.
struct VolumeAlert {
  std::string symbol;
  int volume;
  int threshold;
};

// Type-safe tagged union for all market events.  Adding a new event type only
// requires adding a new struct and extending the variant — no virtual methods.
using MarketEvent = std::variant<PriceChanged, VolumeAlert>;

// ============================================================================
// Section 3: Overloaded Helper (CTAD + fold expressions, C++17 idiom)
// ============================================================================

// Combines multiple lambdas into one callable object so that std::visit can
// dispatch on the active alternative of a std::variant at compile time.
// Each lambda handles exactly one alternative type.
template <typename... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

// C++17 Class Template Argument Deduction guide — allows writing
//   Overloaded{ lambda1, lambda2 }
// without explicitly specifying template arguments.
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

// ============================================================================
// Section 4: Observer Interface
// ============================================================================

class IObserver {
 public:
  virtual ~IObserver() = default;

  // Called by the subject when a MarketEvent occurs.
  virtual void OnNotify(const MarketEvent& event) = 0;

  // Returns the observer's name (used for identification and unsubscription).
  [[nodiscard]] virtual std::string_view Name() const = 0;
};

// ============================================================================
// Section 5: Subject — StockMarket (thread-safe observer management)
// ============================================================================

class StockMarket {
 public:
  // Register an observer.  Uses an exclusive (write) lock.
  void Subscribe(const std::shared_ptr<IObserver>& observer)
  {
    std::unique_lock lock(mutex_);
    observers_.push_back(observer);
  }

  // Unregister an observer by name.  Uses an exclusive (write) lock.
  void Unsubscribe(std::string_view name)
  {
    std::unique_lock lock(mutex_);
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
                        [name](const std::shared_ptr<IObserver>& obs) {
                          return obs->Name() == name;
                        }),
        observers_.end());
  }

  // Update a stock price and notify all observers.
  // Uses std::optional to represent the absence of a previous price (first quote).
  void UpdatePrice(std::string_view symbol, double new_price)
  {
    std::optional<double> old_price;
    {
      // Exclusive lock on price data.
      std::unique_lock lock(data_mutex_);
      auto it = prices_.find(std::string(symbol));
      if (it != prices_.end()) {
        old_price = it->second;
      }
      prices_[std::string(symbol)] = new_price;
    }

    // Build the event — old_price defaults to kDefaultPrice if this is the
    // first quote for the symbol.
    MarketEvent event = PriceChanged{
        std::string(symbol),
        old_price.value_or(kDefaultPrice),
        new_price};
    NotifyAll(event);
  }

  // Check if volume exceeds the threshold; if so, notify observers.
  void CheckVolume(std::string_view symbol, int volume, int threshold)
  {
    if (volume >= threshold) {
      MarketEvent event = VolumeAlert{
          std::string(symbol),
          volume,
          threshold};
      NotifyAll(event);
    }
  }

  // Return the current number of registered observers.
  [[nodiscard]] size_t ObserverCount() const
  {
    std::shared_lock lock(mutex_);
    return observers_.size();
  }

 private:
  // Notify every registered observer.  Uses a shared (read) lock so that
  // multiple threads can iterate concurrently, but Subscribe/Unsubscribe
  // (which modify the vector) block until iteration completes.
  void NotifyAll(const MarketEvent& event)
  {
    std::shared_lock lock(mutex_);
    for (const auto& observer : observers_) {
      observer->OnNotify(event);
    }
  }

  std::vector<std::shared_ptr<IObserver>> observers_;
  mutable std::shared_mutex mutex_;       // guards observers_
  mutable std::shared_mutex data_mutex_;  // guards prices_
  std::unordered_map<std::string, double> prices_;
};

// ============================================================================
// Section 6: Concrete Observer — PriceDisplay
// ============================================================================

// Displays every incoming event to stdout.  Uses std::visit + Overloaded to
// dispatch on the event variant at compile time — no dynamic_cast needed.
class PriceDisplay : public IObserver {
 public:
  explicit PriceDisplay(std::string name) : name_(std::move(name)) {}

  void OnNotify(const MarketEvent& event) override
  {
    // std::visit with Overloaded: each lambda handles one variant alternative.
    std::visit(
        Overloaded{
            [this](const PriceChanged& e) {
              double change_pct = 0.0;
              if (e.old_price != kDefaultPrice) {
                change_pct = ((e.new_price - e.old_price) / e.old_price) * 100.0;
              }
              std::cout << "  [" << name_ << "] "
                        << e.symbol << ": "
                        << std::fixed << std::setprecision(2)
                        << e.old_price << " -> " << e.new_price;
              if (change_pct >= 0.0) {
                std::cout << " (+" << change_pct << "%)";
              } else {
                std::cout << " (" << change_pct << "%)";
              }
              std::cout << "\n";
            },
            [this](const VolumeAlert& e) {
              std::cout << "  [" << name_ << "] "
                        << "VOLUME ALERT: " << e.symbol
                        << " volume=" << e.volume
                        << " >= threshold=" << e.threshold << "\n";
            }},
        event);
  }

  [[nodiscard]] std::string_view Name() const override { return name_; }

 private:
  std::string name_;
};

// ============================================================================
// Section 7: Concrete Observer — RiskManager
// ============================================================================

// Only reacts to large price swings.  Demonstrates that different observers
// can interpret the same event differently.
class RiskManager : public IObserver {
 public:
  RiskManager(std::string name, double alert_threshold_pct)
      : name_(std::move(name)),
        alert_threshold_pct_(alert_threshold_pct) {}

  void OnNotify(const MarketEvent& event) override
  {
    std::visit(
        Overloaded{
            [this](const PriceChanged& e) {
              // Skip the first quote (no previous price to compare).
              if (e.old_price == kDefaultPrice) {
                return;
              }
              double change_pct = std::abs(
                  ((e.new_price - e.old_price) / e.old_price) * 100.0);
              if (change_pct >= alert_threshold_pct_) {
                std::cout << "  [" << name_ << "] *** RISK ALERT *** "
                          << e.symbol << " moved "
                          << std::fixed << std::setprecision(2)
                          << change_pct << "% (threshold: "
                          << alert_threshold_pct_ << "%)\n";
              }
            },
            [this](const VolumeAlert& e) {
              std::cout << "  [" << name_ << "] "
                        << "Volume risk: " << e.symbol
                        << " volume=" << e.volume << "\n";
            }},
        event);
  }

  [[nodiscard]] std::string_view Name() const override { return name_; }

 private:
  std::string name_;
  double alert_threshold_pct_;
};

}  // namespace observer

// ============================================================================
// Section 8: Demonstration
// ============================================================================

int main()
{
  using namespace observer;

  // --- Create the subject (the stock market data feed) ---
  StockMarket market;

  // --- Create concrete observers ---
  auto display1 = std::make_shared<PriceDisplay>("TradingView");
  auto display2 = std::make_shared<PriceDisplay>("Bloomberg");
  auto risk_engine = std::make_shared<RiskManager>("RiskEngine", 5.0);

  // --- Subscribe observers to the subject ---
  market.Subscribe(display1);
  market.Subscribe(display2);
  market.Subscribe(risk_engine);

  std::cout << "========================================\n"
            << " Observer Pattern: Stock Market Tracking\n"
            << "========================================\n\n";
  std::cout << "Active observers: " << market.ObserverCount() << "\n\n";

  // --- Scenario 1: First quote for a symbol (std::optional: no old price) ---
  std::cout << "--- AAPL: first quote at 150.00 ---\n";
  market.UpdatePrice("AAPL", 150.00);

  // --- Scenario 2: Small price change (below risk threshold) ---
  std::cout << "\n--- AAPL: 150.00 -> 155.00 (+3.3%) ---\n";
  market.UpdatePrice("AAPL", 155.00);

  // --- Scenario 3: Large price change (triggers risk alert) ---
  std::cout << "\n--- AAPL: 155.00 -> 175.00 (+12.9%) ---\n";
  market.UpdatePrice("AAPL", 175.00);

  // --- Scenario 4: Volume alert ---
  std::cout << "\n--- TSLA: volume=50000, threshold=30000 ---\n";
  market.CheckVolume("TSLA", 50000, 30000);

  // --- Scenario 5: Unsubscribe one observer, then notify ---
  std::cout << "\n--- Unsubscribing Bloomberg ---\n";
  market.Unsubscribe("Bloomberg");
  std::cout << "Active observers: " << market.ObserverCount() << "\n\n";

  std::cout << "--- GOOGL: 2800.00 -> 2900.00 (+3.6%) ---\n";
  market.UpdatePrice("GOOGL", 2900.00);

  // --- Scenario 6: First quote for a new symbol (optional old_price) ---
  std::cout << "\n--- NVDA: first quote at 800.00 ---\n";
  market.UpdatePrice("NVDA", 800.00);

  // --- Scenario 7: Drastic drop (large negative change) ---
  std::cout << "\n--- NVDA: 800.00 -> 700.00 (-12.5%) ---\n";
  market.UpdatePrice("NVDA", 700.00);

  return 0;
}
