/**
 * Prototype Pattern (原型模式)
 *
 * Intent:  Create new objects by cloning existing prototypes, avoiding
 *          expensive construction and decoupling clients from concrete classes.
 *
 * C++17 Features Used:
 *   - std::unique_ptr for ownership-safe clone return types
 *   - std::optional for safe prototype lookup in registry
 *   - if constexpr for compile-time clone strategy dispatch
 *   - std::string_view to avoid unnecessary string copies
 *   - structured bindings for registry iteration
 *   - inline variables for constant definitions
 *   - fold expressions for variadic debug printing
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

inline constexpr int kDefaultHealth = 100;
inline constexpr double kDefaultDamage = 25.0;

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

class Prototype;

// ---------------------------------------------------------------------------
// Helper: variadic debug printer using fold expression
// ---------------------------------------------------------------------------

template <typename... Args>
void PrintLn(Args&&... args)
{
    (std::cout << ... << std::forward<Args>(args));
    std::cout << '\n';
}

// ---------------------------------------------------------------------------
// Prototype interface
// ---------------------------------------------------------------------------

class Prototype
{
public:
    virtual ~Prototype() = default;

    virtual std::unique_ptr<Prototype> Clone() const = 0;
    virtual std::string ToString() const = 0;
};

// ---------------------------------------------------------------------------
// ConcretePrototypeA  --  a game enemy with nested attributes
// ---------------------------------------------------------------------------

class ConcretePrototypeA : public Prototype
{
public:
    ConcretePrototypeA(std::string_view name,
                       int health,
                       std::vector<std::string> abilities)
        : name_(name),
          health_(health),
          abilities_(std::move(abilities))
    {
    }

    std::unique_ptr<Prototype> Clone() const override
    {
        // Deep copy: vector is copied value-by-value automatically.
        return std::make_unique<ConcretePrototypeA>(name_, health_, abilities_);
    }

    std::string ToString() const override
    {
        std::string result = "Enemy{name=\"" + name_ +
                             "\", health=" + std::to_string(health_) +
                             ", abilities=[";
        for (size_t i = 0; i < abilities_.size(); ++i)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += abilities_[i];
        }
        result += "]}";
        return result;
    }

    void SetHealth(int health)
    {
        health_ = health;
    }

    void AddAbility(std::string_view ability)
    {
        abilities_.emplace_back(ability);
    }

private:
    std::string name_;
    int health_;
    std::vector<std::string> abilities_;
};

// ---------------------------------------------------------------------------
// ConcretePrototypeB  --  a terrain tile with variant properties
// ---------------------------------------------------------------------------

class ConcretePrototypeB : public Prototype
{
public:
    ConcretePrototypeB(std::string_view label, double walk_speed)
        : label_(label),
          walk_speed_(walk_speed)
    {
    }

    std::unique_ptr<Prototype> Clone() const override
    {
        return std::make_unique<ConcretePrototypeB>(label_, walk_speed_);
    }

    std::string ToString() const override
    {
        return "Terrain{label=\"" + label_ +
               "\", walk_speed=" + std::to_string(walk_speed_) + "}";
    }

    void SetWalkSpeed(double speed)
    {
        walk_speed_ = speed;
    }

private:
    std::string label_;
    double walk_speed_;
};

// ---------------------------------------------------------------------------
// PrototypeRegistry  --  stores named prototypes, returns clones via optional
// ---------------------------------------------------------------------------

class PrototypeRegistry
{
public:
    void Register(std::string_view name, std::unique_ptr<Prototype> proto)
    {
        prototypes_[std::string(name)] = std::move(proto);
    }

    // Returns std::nullopt when the requested name is not found.
    [[nodiscard]] std::optional<std::unique_ptr<Prototype>>
    Create(std::string_view name) const
    {
        auto it = prototypes_.find(std::string(name));
        if (it == prototypes_.end())
        {
            return std::nullopt;
        }
        return it->second->Clone();
    }

    // Iterate with structured bindings for debugging.
    void PrintAll() const
    {
        for (const auto& [name, proto] : prototypes_)
        {
            PrintLn("  [", name, "] => ", proto->ToString());
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Prototype>> prototypes_;
};

// ---------------------------------------------------------------------------
// Generic clone helper using if constexpr
// ---------------------------------------------------------------------------

// A simple trait to detect whether a type is a Prototype.
template <typename T>
inline constexpr bool kIsPrototype = std::is_base_of_v<Prototype, T>;

// CloneOrCopy: if T derives from Prototype, call Clone(); otherwise copy.
template <typename T>
auto CloneOrCopy(const T& obj)
{
    if constexpr (kIsPrototype<T>)
    {
        // Prototype hierarchy -- use virtual Clone().
        return obj.Clone();
    }
    else
    {
        // Plain value type -- just copy.
        return obj;
    }
}

// ---------------------------------------------------------------------------
// main  --  demonstrate the Prototype pattern
// ---------------------------------------------------------------------------

int main()
{
    PrintLn("=== Prototype Pattern Demo ===\n");

    // --- 1. Direct cloning without a registry ---
    PrintLn("--- Direct Clone ---");

    auto goblin = std::make_unique<ConcretePrototypeA>(
        "Goblin", kDefaultHealth, std::vector<std::string>{"Slash", "Hide"});

    auto goblin_clone = goblin->Clone();

    PrintLn("Original : ", goblin->ToString());
    PrintLn("Clone    : ", goblin_clone->ToString());

    // Mutate the clone -- original must stay unchanged.
    auto* goblin_ptr = dynamic_cast<ConcretePrototypeA*>(goblin_clone.get());
    goblin_ptr->SetHealth(80);
    goblin_ptr->AddAbility("Fireball");

    PrintLn("\nAfter mutating clone:");
    PrintLn("Original : ", goblin->ToString());
    PrintLn("Clone    : ", goblin_clone->ToString());

    // --- 2. Registry-based cloning ---
    PrintLn("\n--- Prototype Registry ---");

    PrototypeRegistry registry;

    // Register prototypes.
    registry.Register(
        "enemy_grunt",
        std::make_unique<ConcretePrototypeA>(
            "Grunt", 150, std::vector<std::string>{"Smash", "Roar"}));

    registry.Register(
        "terrain_grass",
        std::make_unique<ConcretePrototypeB>("Grassland", 1.0));

    registry.Register(
        "terrain_swamp",
        std::make_unique<ConcretePrototypeB>("Swamp", 0.4));

    PrintLn("Registered prototypes:");
    registry.PrintAll();

    // Create instances from the registry using structured binding on optional.
    PrintLn("\n--- Creating from Registry ---");

    if (auto obj = registry.Create("enemy_grunt"); obj.has_value())
    {
        PrintLn("Spawned : ", obj.value()->ToString());
    }

    if (auto obj = registry.Create("terrain_swamp"); obj.has_value())
    {
        PrintLn("Spawned : ", obj.value()->ToString());
    }

    // Lookup a missing key -- demonstrates std::optional.
    if (auto obj = registry.Create("nonexistent"); !obj.has_value())
    {
        PrintLn("Lookup \"nonexistent\" => not found (std::nullopt)");
    }

    // --- 3. if constexpr helper demo ---
    PrintLn("\n--- if constexpr CloneOrCopy ---");

    ConcretePrototypeB swamp("DeepSwamp", 0.2);
    auto swamp_copy = CloneOrCopy(swamp);   // Uses Clone() path.
    PrintLn("Original : ", swamp.ToString());
    PrintLn("Cloned   : ", swamp_copy->ToString());

    int plain_value = 42;
    auto copied_value = CloneOrCopy(plain_value);  // Uses copy path.
    PrintLn("Original int : ", plain_value);
    PrintLn("Copied int   : ", copied_value);

    PrintLn("\n=== End of Prototype Demo ===");
    return 0;
}
