// Factory Method Pattern (工厂方法模式)
// Intent: Define an interface for creating an object, letting subclasses decide
// which class to instantiate. Factory Method lets a class defer instantiation
// to subclasses.
//
// C++17 features demonstrated:
//   - std::unique_ptr for automatic product lifetime management
//   - std::optional to express "creation may fail"
//   - std::variant + std::visit for type-safe product collections
//   - std::string_view for zero-copy string parameters
//   - if constexpr for compile-time factory dispatch
//   - inline variables for global constants
//   - [[nodiscard]] to prevent ignoring return values
//   - Structured bindings

#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

// ============================================================================
// 1. Product interface and concrete products
// ============================================================================

/// Abstract product — the interface all transport types must implement.
class Transport
{
 public:
  virtual ~Transport() = default;

  /// Perform the delivery operation.
  [[nodiscard]] virtual std::string Deliver() const = 0;

  /// Return a human-readable type name.
  [[nodiscard]] virtual std::string_view GetType() const = 0;
};

/// Concrete product — road transport via truck.
class Truck : public Transport
{
 public:
  [[nodiscard]] std::string Deliver() const override
  {
    return "Delivering by land in a box (Truck)";
  }

  [[nodiscard]] std::string_view GetType() const override
  {
    return "Truck";
  }
};

/// Concrete product — sea transport via ship.
class Ship : public Transport
{
 public:
  [[nodiscard]] std::string Deliver() const override
  {
    return "Delivering by sea in a container (Ship)";
  }

  [[nodiscard]] std::string_view GetType() const override
  {
    return "Ship";
  }
};

/// Concrete product — air transport via airplane.
class Airplane : public Transport
{
 public:
  [[nodiscard]] std::string Deliver() const override
  {
    return "Delivering by air in a cargo hold (Airplane)";
  }

  [[nodiscard]] std::string_view GetType() const override
  {
    return "Airplane";
  }
};

// ============================================================================
// 2. Abstract Creator — declares the factory method
// ============================================================================

/// Abstract creator. Defines the factory method and contains business logic
/// that works with the product through its abstract interface.
class Creator
{
 public:
  virtual ~Creator() = default;

  /// The Factory Method — subclasses override this to produce their product.
  [[nodiscard]] virtual std::unique_ptr<Transport> CreateTransport() const = 0;

  /// Business logic that depends on the product, but not on its concrete type.
  void PlanDelivery() const
  {
    auto transport = CreateTransport();
    std::cout << "  Creator: Planning delivery via "
              << transport->GetType() << "...\n";
    std::cout << "  Logistics: " << transport->Deliver() << "\n";
  }
};

// ============================================================================
// 3. Concrete Creators — override the factory method
// ============================================================================

class RoadLogistics : public Creator
{
 public:
  [[nodiscard]] std::unique_ptr<Transport> CreateTransport() const override
  {
    return std::make_unique<Truck>();
  }
};

class SeaLogistics : public Creator
{
 public:
  [[nodiscard]] std::unique_ptr<Transport> CreateTransport() const override
  {
    return std::make_unique<Ship>();
  }
};

class AirLogistics : public Creator
{
 public:
  [[nodiscard]] std::unique_ptr<Transport> CreateTransport() const override
  {
    return std::make_unique<Airplane>();
  }
};

// ============================================================================
// 4. Modern C++17: Registry-based factory with std::optional
// ============================================================================

/// A factory that registers transport creators by name and uses
/// std::optional to safely express "key not found" without returning nullptr.
class TransportFactory
{
 public:
  /// Register a new transport type. The creator is a callable that returns
  /// a unique_ptr<Transport>.
  void RegisterTransport(std::string_view name,
                         std::function<std::unique_ptr<Transport>()> creator)
  {
    creators_.emplace(std::string(name), std::move(creator));
  }

  /// Try to create a transport by name. Returns std::nullopt if the name
  /// is not registered — no nullptr, no exceptions, just clean semantics.
  [[nodiscard]] std::optional<std::unique_ptr<Transport>> Create(
      std::string_view name) const
  {
    auto it = creators_.find(std::string(name));
    if (it == creators_.end())
    {
      return std::nullopt;
    }
    return it->second();
  }

  /// Compile-time dispatch using if constexpr. The template parameter selects
  /// which concrete logistics creator to use, resolved entirely at compile time.
  template <typename LogisticsType>
  [[nodiscard]] std::optional<std::unique_ptr<Transport>> Create() const
  {
    if constexpr (std::is_same_v<LogisticsType, RoadLogistics>)
    {
      return Create("Truck");
    }
    else if constexpr (std::is_same_v<LogisticsType, SeaLogistics>)
    {
      return Create("Ship");
    }
    else if constexpr (std::is_same_v<LogisticsType, AirLogistics>)
    {
      return Create("Airplane");
    }
    else
    {
      return std::nullopt;
    }
  }

  /// Return the list of registered transport names.
  [[nodiscard]] std::vector<std::string> AvailableTypes() const
  {
    std::vector<std::string> names;
    names.reserve(creators_.size());
    for (const auto& [name, _] : creators_)
    {
      names.push_back(name);
    }
    return names;
  }

 private:
  std::unordered_map<std::string, std::function<std::unique_ptr<Transport>()>>
      creators_;
};

// ============================================================================
// 5. Modern C++17: std::variant-based creation
// ============================================================================

/// A variant that can hold any concrete transport type.
using TransportVariant = std::variant<Truck, Ship, Airplane>;

/// Create a transport variant by name. Uses structured binding-friendly
/// return type and string_view for zero-copy parameter passing.
[[nodiscard]] std::optional<TransportVariant> CreateTransportVariant(
    std::string_view name)
{
  static const std::unordered_map<std::string, TransportVariant> kPrototypes = {
      {"Truck", Truck{}},
      {"Ship", Ship{}},
      {"Airplane", Airplane{}},
  };

  auto it = kPrototypes.find(std::string(name));
  if (it == kPrototypes.end())
  {
    return std::nullopt;
  }
  return it->second;
}

// ============================================================================
// 6. Demo helpers
// ============================================================================

inline constexpr int kSeparatorWidth = 60;

void PrintSeparator(std::string_view title)
{
  std::cout << "\n" << std::string(kSeparatorWidth, '=') << "\n";
  std::cout << " " << title << "\n";
  std::cout << std::string(kSeparatorWidth, '=') << "\n";
}

// ============================================================================
// 7. Demonstration
// ============================================================================

/// Classic Factory Method — each subclass produces its own transport.
void DemoClassicFactoryMethod()
{
  PrintSeparator("Classic Factory Method (Subclass Dispatch)");

  // The client code works with the abstract Creator interface.
  // It never knows which concrete product it gets.
  const std::unique_ptr<Creator> logistics[] = {
      std::make_unique<RoadLogistics>(),
      std::make_unique<SeaLogistics>(),
      std::make_unique<AirLogistics>(),
  };

  for (const auto& logi : logistics)
  {
    logi->PlanDelivery();
    std::cout << "\n";
  }
}

/// Modern registry-based factory with std::optional.
void DemoRegistryFactory()
{
  PrintSeparator("Modern Factory: Registry + std::optional");

  TransportFactory factory;
  factory.RegisterTransport("Truck", [] { return std::make_unique<Truck>(); });
  factory.RegisterTransport("Ship", [] { return std::make_unique<Ship>(); });
  factory.RegisterTransport("Airplane",
                            [] { return std::make_unique<Airplane>(); });

  std::cout << "Available types: ";
  for (const auto& name : factory.AvailableTypes())
  {
    std::cout << name << " ";
  }
  std::cout << "\n\n";

  // Attempt to create known and unknown transports.
  for (const std::string_view name : {"Truck", "Ship", "Bicycle"})
  {
    auto result = factory.Create(name);
    if (result.has_value())
    {
      std::cout << "  Created [" << name << "]: "
                << (*result)->Deliver() << "\n";
    }
    else
    {
      std::cout << "  [" << name << "]: not registered — creation skipped\n";
    }
  }

  // Compile-time dispatch via if constexpr.
  std::cout << "\n  Compile-time dispatch (if constexpr):\n";
  auto sea = factory.Create<SeaLogistics>();
  if (sea.has_value())
  {
    std::cout << "    SeaLogistics -> " << (*sea)->Deliver() << "\n";
  }
}

/// std::variant-based creation with std::visit.
void DemoVariantFactory()
{
  PrintSeparator("Modern Factory: std::variant + std::visit");

  for (const std::string_view name : {"Truck", "Ship", "Airplane"})
  {
    auto variant = CreateTransportVariant(name);
    if (!variant.has_value())
    {
      std::cout << "  [" << name << "]: unknown type\n";
      continue;
    }

    // std::visit applies the visitor to the active variant alternative.
    std::visit(
        [](const auto& transport)
        {
          std::cout << "  Variant [" << transport.GetType() << "]: "
                    << transport.Deliver() << "\n";
        },
        *variant);
  }
}

// ============================================================================
// main
// ============================================================================

int main()
{
  std::cout << std::string(kSeparatorWidth, '*') << "\n";
  std::cout << " Factory Method Pattern — C++17 Demonstration\n";
  std::cout << std::string(kSeparatorWidth, '*') << "\n";

  DemoClassicFactoryMethod();
  DemoRegistryFactory();
  DemoVariantFactory();

  std::cout << "\n" << std::string(kSeparatorWidth, '*') << "\n";
  std::cout << " All demonstrations complete.\n";
  std::cout << std::string(kSeparatorWidth, '*') << "\n";

  return 0;
}
