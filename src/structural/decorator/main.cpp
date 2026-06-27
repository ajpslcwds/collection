/**
 * Decorator Pattern - C++17 Implementation
 *
 * Intent: Dynamically attach additional responsibilities to objects.
 * Provides a flexible alternative to subclassing for extending functionality.
 *
 * C++17 Features Used:
 *   - std::unique_ptr for automatic memory management
 *   - std::optional for optional configuration
 *   - std::string_view for efficient string handling
 *   - Structured bindings for clean return values
 *   - [[nodiscard]] attribute for value returns
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// ============================================================================
// Component Interface
// ============================================================================

class Beverage
{
public:
  virtual ~Beverage() = default;

  [[nodiscard]] virtual std::string GetDescription() const = 0;
  [[nodiscard]] virtual double GetCost() const = 0;
};

// ============================================================================
// Concrete Component
// ============================================================================

class Espresso : public Beverage
{
public:
  [[nodiscard]] std::string GetDescription() const override
  {
    return "Espresso";
  }

  [[nodiscard]] double GetCost() const override
  {
    return 1.99;
  }
};

class HouseBlend : public Beverage
{
public:
  [[nodiscard]] std::string GetDescription() const override
  {
    return "House Blend Coffee";
  }

  [[nodiscard]] double GetCost() const override
  {
    return 0.89;
  }
};

class DarkRoast : public Beverage
{
public:
  [[nodiscard]] std::string GetDescription() const override
  {
    return "Dark Roast Coffee";
  }

  [[nodiscard]] double GetCost() const override
  {
    return 0.99;
  }
};

// ============================================================================
// Abstract Decorator
// ============================================================================

class CondimentDecorator : public Beverage
{
public:
  explicit CondimentDecorator(std::unique_ptr<Beverage> beverage)
      : beverage_(std::move(beverage))
  {
  }

  [[nodiscard]] std::string GetDescription() const override
  {
    return beverage_->GetDescription();
  }

  [[nodiscard]] double GetCost() const override
  {
    return beverage_->GetCost();
  }

protected:
  [[nodiscard]] const Beverage& GetBeverage() const
  {
    return *beverage_;
  }

private:
  std::unique_ptr<Beverage> beverage_;
};

// ============================================================================
// Concrete Decorators
// ============================================================================

class Milk : public CondimentDecorator
{
public:
  Milk(std::unique_ptr<Beverage> beverage, std::optional<double> cost = std::nullopt)
      : CondimentDecorator(std::move(beverage)),
        cost_(cost.value_or(kDefaultCost))
  {
  }

  [[nodiscard]] std::string GetDescription() const override
  {
    return GetBeverage().GetDescription() + ", Milk";
  }

  [[nodiscard]] double GetCost() const override
  {
    return GetBeverage().GetCost() + cost_;
  }

private:
  static constexpr double kDefaultCost = 0.10;
  double cost_;
};

class Mocha : public CondimentDecorator
{
public:
  explicit Mocha(std::unique_ptr<Beverage> beverage)
      : CondimentDecorator(std::move(beverage))
  {
  }

  [[nodiscard]] std::string GetDescription() const override
  {
    return GetBeverage().GetDescription() + ", Mocha";
  }

  [[nodiscard]] double GetCost() const override
  {
    return GetBeverage().GetCost() + kCost;
  }

private:
  static constexpr double kCost = 0.20;
};

class Soy : public CondimentDecorator
{
public:
  Soy(std::unique_ptr<Beverage> beverage, std::optional<std::string_view> size = std::nullopt)
      : CondimentDecorator(std::move(beverage)),
        size_(size.value_or(kDefaultSize))
  {
  }

  [[nodiscard]] std::string GetDescription() const override
  {
    return GetBeverage().GetDescription() + ", Soy";
  }

  [[nodiscard]] double GetCost() const override
  {
    return GetBeverage().GetCost() + kCost;
  }

  [[nodiscard]] std::string_view GetSize() const
  {
    return size_;
  }

private:
  static constexpr double kCost = 0.15;
  static constexpr std::string_view kDefaultSize = "medium";
  std::string_view size_;
};

class Whip : public CondimentDecorator
{
public:
  explicit Whip(std::unique_ptr<Beverage> beverage)
      : CondimentDecorator(std::move(beverage))
  {
  }

  [[nodiscard]] std::string GetDescription() const override
  {
    return GetBeverage().GetDescription() + ", Whip";
  }

  [[nodiscard]] double GetCost() const override
  {
    return GetBeverage().GetCost() + kCost;
  }

private:
  static constexpr double kCost = 0.10;
};

// ============================================================================
// Helper Functions
// ============================================================================

void PrintOrder(std::string_view label, const Beverage& beverage)
{
  std::cout << label << "\n"
            << "  Description: " << beverage.GetDescription() << "\n"
            << "  Cost: $" << beverage.GetCost() << "\n\n";
}

// ============================================================================
// Main
// ============================================================================

int main()
{
  // Order 1: Simple Espresso
  auto espresso = std::make_unique<Espresso>();
  PrintOrder("Order 1:", *espresso);

  // Order 2: Dark Roast with double Mocha and Whip
  auto dark_roast = std::make_unique<DarkRoast>();
  auto mocha1 = std::make_unique<Mocha>(std::move(dark_roast));
  auto mocha2 = std::make_unique<Mocha>(std::move(mocha1));
  auto whip = std::make_unique<Whip>(std::move(mocha2));
  PrintOrder("Order 2:", *whip);

  // Order 3: House Blend with Soy, Mocha, and Whip
  auto house_blend = std::make_unique<HouseBlend>();
  auto soy = std::make_unique<Soy>(std::move(house_blend), "large");
  auto mocha = std::make_unique<Mocha>(std::move(soy));
  auto whip2 = std::make_unique<Whip>(std::move(mocha));
  PrintOrder("Order 3:", *whip2);

  // Order 4: Espresso with custom-priced Milk
  auto espresso2 = std::make_unique<Espresso>();
  auto milk = std::make_unique<Milk>(std::move(espresso2), 0.15);
  PrintOrder("Order 4:", *milk);

  return 0;
}
