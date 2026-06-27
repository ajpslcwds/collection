/**
 * Builder Pattern - C++17 Implementation
 *
 * Intent: Separate the construction of a complex object from its representation,
 * allowing the same construction process to create different representations.
 *
 * C++17 Features Used:
 * - std::optional for optional components
 * - std::variant for type-safe material choices
 * - std::string_view for efficient string parameters
 * - inline variables for class constants
 * - structured bindings for cleaner result handling
 * - if constexpr for compile-time optimization
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>
#include <memory>

namespace builder {

// Material types using std::variant for type safety
struct Wood {
    std::string type;
};

struct Brick {
    std::string color;
};

struct Steel {
    int grade;
};

using Material = std::variant<Wood, Brick, Steel>;

// Room structure with optional luxury features
struct Room {
    std::string name;
    double area;
    std::optional<bool> has_balcony;
    std::optional<std::string> floor_type;
};

// House is the complex product
class House {
public:
    void SetFoundation(std::string_view foundation_type)
    {
        foundation_ = foundation_type;
    }

    void SetStructure(const Material& material)
    {
        structure_ = material;
    }

    void SetRoof(std::string_view roof_type)
    {
        roof_ = roof_type;
    }

    void AddRoom(const Room& room)
    {
        rooms_.push_back(room);
    }

    void SetGarage(bool has_garage)
    {
        has_garage_ = has_garage;
    }

    void SetGarden(bool has_garden)
    {
        has_garden_ = has_garden;
    }

    void SetSwimmingPool(bool has_pool)
    {
        has_pool_ = has_pool;
    }

    const std::vector<Room>& GetRooms() const
    {
        return rooms_;
    }

    void Display() const
    {
        std::cout << "\n=== House Details ===\n";
        std::cout << "Foundation: " << foundation_ << "\n";

        // Using std::visit with structured binding to display material info
        std::visit([](const auto& material) {
            using T = std::decay_t<decltype(material)>;
            if constexpr (std::is_same_v<T, Wood>) {
                std::cout << "Structure: Wood (" << material.type << ")\n";
            } else if constexpr (std::is_same_v<T, Brick>) {
                std::cout << "Structure: Brick (" << material.color << ")\n";
            } else if constexpr (std::is_same_v<T, Steel>) {
                std::cout << "Structure: Steel (Grade " << material.grade << ")\n";
            }
        }, structure_);

        std::cout << "Roof: " << roof_ << "\n";

        std::cout << "\nRooms (" << rooms_.size() << "):\n";
        for (const auto& room : rooms_) {
            std::cout << "  - " << room.name << " (" << room.area << " sqm)";
            if (room.has_balcony && *room.has_balcony) {
                std::cout << " [Balcony]";
            }
            if (room.floor_type) {
                std::cout << " [" << *room.floor_type << " floor]";
            }
            std::cout << "\n";
        }

        std::cout << "\nAmenities:\n";
        std::cout << "  Garage: " << (has_garage_ ? "Yes" : "No") << "\n";
        std::cout << "  Garden: " << (has_garden_ ? "Yes" : "No") << "\n";
        std::cout << "  Swimming Pool: " << (has_pool_ ? "Yes" : "No") << "\n";
        std::cout << "==================\n";
    }

private:
    std::string foundation_;
    Material structure_;
    std::string roof_;
    std::vector<Room> rooms_;
    bool has_garage_{false};
    bool has_garden_{false};
    bool has_pool_{false};
};

// Abstract Builder interface
class HouseBuilder {
public:
    virtual ~HouseBuilder() = default;

    virtual void BuildFoundation() = 0;
    virtual void BuildStructure() = 0;
    virtual void BuildRoof() = 0;
    virtual void BuildRooms() = 0;
    virtual void BuildAmenities() = 0;

    std::unique_ptr<House> GetResult()
    {
        return std::move(house_);
    }

protected:
    std::unique_ptr<House> house_{std::make_unique<House>()};
};

// Concrete Builder: Modern House
class ModernHouseBuilder : public HouseBuilder {
public:
    void BuildFoundation() override
    {
        house_->SetFoundation("Reinforced Concrete");
    }

    void BuildStructure() override
    {
        house_->SetStructure(Steel{3});
    }

    void BuildRoof() override
    {
        house_->SetRoof("Flat Roof");
    }

    void BuildRooms() override
    {
        house_->AddRoom({"Living Room", 45.0, true, "Hardwood"});
        house_->AddRoom({"Kitchen", 25.0, std::nullopt, "Tile"});
        house_->AddRoom({"Master Bedroom", 35.0, true, "Hardwood"});
        house_->AddRoom({"Bathroom", 15.0, std::nullopt, "Marble"});
    }

    void BuildAmenities() override
    {
        house_->SetGarage(true);
        house_->SetGarden(true);
        house_->SetSwimmingPool(true);
    }
};

// Concrete Builder: Traditional House
class TraditionalHouseBuilder : public HouseBuilder {
public:
    void BuildFoundation() override
    {
        house_->SetFoundation("Stone Foundation");
    }

    void BuildStructure() override
    {
        house_->SetStructure(Brick{"Red"});
    }

    void BuildRoof() override
    {
        house_->SetRoof("Gabled Roof");
    }

    void BuildRooms() override
    {
        house_->AddRoom({"Parlor", 30.0, std::nullopt, "Hardwood"});
        house_->AddRoom({"Kitchen", 20.0, std::nullopt, "Tile"});
        house_->AddRoom({"Bedroom", 25.0, std::nullopt, "Carpet"});
        house_->AddRoom({"Study", 20.0, std::nullopt, "Hardwood"});
    }

    void BuildAmenities() override
    {
        house_->SetGarage(false);
        house_->SetGarden(true);
        house_->SetSwimmingPool(false);
    }
};

// Director class that orchestrates the building process
class HouseDirector {
public:
    void SetBuilder(std::unique_ptr<HouseBuilder> builder)
    {
        builder_ = std::move(builder);
    }

    std::unique_ptr<House> Construct()
    {
        if (!builder_) {
            return nullptr;
        }

        builder_->BuildFoundation();
        builder_->BuildStructure();
        builder_->BuildRoof();
        builder_->BuildRooms();
        builder_->BuildAmenities();

        return builder_->GetResult();
    }

private:
    std::unique_ptr<HouseBuilder> builder_;
};

}  // namespace builder

// Helper function to get material name using std::visit
std::string GetMaterialName(const builder::Material& material)
{
    return std::visit([](const auto& m) -> std::string {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, builder::Wood>) {
            return "Wood";
        } else if constexpr (std::is_same_v<T, builder::Brick>) {
            return "Brick";
        } else if constexpr (std::is_same_v<T, builder::Steel>) {
            return "Steel";
        }
    }, material);
}

int main()
{
    std::cout << "Builder Pattern - C++17 Implementation\n";
    std::cout << "=======================================\n";

    // Example 1: Build a modern house using Director
    std::cout << "\n[Example 1] Building Modern House with Director\n";

    builder::HouseDirector director;
    director.SetBuilder(std::make_unique<builder::ModernHouseBuilder>());
    auto modern_house = director.Construct();

    if (modern_house) {
        modern_house->Display();
    }

    // Example 2: Build a traditional house using Director
    std::cout << "\n[Example 2] Building Traditional House with Director\n";

    director.SetBuilder(std::make_unique<builder::TraditionalHouseBuilder>());
    auto traditional_house = director.Construct();

    if (traditional_house) {
        traditional_house->Display();
    }

    // Example 3: Direct Builder usage without Director (fluent interface style)
    std::cout << "\n[Example 3] Direct Builder Usage (Fluent Interface)\n";

    builder::House custom_house;
    custom_house.SetFoundation("Pile Foundation");
    custom_house.SetStructure(builder::Wood{"Oak"});
    custom_house.SetRoof("Hip Roof");
    custom_house.AddRoom({"Living Room", 50.0, true, "Bamboo"});
    custom_house.AddRoom({"Kitchen", 30.0, std::nullopt, "Tile"});
    custom_house.AddRoom({"Bedroom", 40.0, true, "Carpet"});
    custom_house.AddRoom({"Home Office", 25.0, std::nullopt, "Laminate"});
    custom_house.SetGarage(true);
    custom_house.SetGarden(true);
    custom_house.SetSwimmingPool(false);

    custom_house.Display();

    // Demonstrate std::optional usage
    std::cout << "\n[std::optional Demo] Checking optional features\n";

    const auto& rooms = custom_house.GetRooms();
    for (size_t i = 0; i < rooms.size(); ++i) {
        const auto& room = rooms[i];
        std::cout << "Room " << (i + 1) << ": " << room.name;

        // Using structured binding with optional
        if (auto balcony = room.has_balcony; balcony.has_value()) {
            std::cout << " | Balcony: " << (*balcony ? "Yes" : "No");
        } else {
            std::cout << " | Balcony: Not specified";
        }

        if (auto floor = room.floor_type; floor.has_value()) {
            std::cout << " | Floor: " << *floor;
        } else {
            std::cout << " | Floor: Default";
        }

        std::cout << "\n";
    }

    return 0;
}
