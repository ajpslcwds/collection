/**
 * Abstract Factory Pattern — C++17 Implementation
 *
 * Intent:
 *   Provide an interface for creating families of related objects
 *   without specifying their concrete classes.
 *
 * C++17 features used:
 *   - std::unique_ptr for automatic lifetime management
 *   - std::variant to represent a tagged union of product types
 *   - std::optional to express nullable factory lookup
 *   - if constexpr for compile-time branching on variant types
 *   - structured bindings for clean map iteration
 *   - inline variables for the global factory registry
 *   - std::string_view for lightweight constant strings
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

// ---------------------------------------------------------------------------
// Abstract Products
// ---------------------------------------------------------------------------

class Button
{
 public:
  virtual ~Button() = default;
  virtual std::string Render() const = 0;
  virtual std::string OnClick() const = 0;
};

class Checkbox
{
 public:
  virtual ~Checkbox() = default;
  virtual std::string Render() const = 0;
};

class TextBox
{
 public:
  virtual ~TextBox() = default;
  virtual std::string Render() const = 0;
};

// A variant that can hold any one of the three abstract product types.
using AnyWidget = std::variant<
    std::unique_ptr<Button>,
    std::unique_ptr<Checkbox>,
    std::unique_ptr<TextBox>>;

// ---------------------------------------------------------------------------
// Concrete Products — Windows
// ---------------------------------------------------------------------------

class WindowsButton : public Button
{
 public:
  std::string Render() const override
  {
    return "[Windows Button]";
  }

  std::string OnClick() const override
  {
    return "Windows button clicked";
  }
};

class WindowsCheckbox : public Checkbox
{
 public:
  std::string Render() const override
  {
    return "[Windows Checkbox]";
  }
};

class WindowsTextBox : public TextBox
{
 public:
  std::string Render() const override
  {
    return "[Windows TextBox]";
  }
};

// ---------------------------------------------------------------------------
// Concrete Products — macOS
// ---------------------------------------------------------------------------

class MacButton : public Button
{
 public:
  std::string Render() const override
  {
    return "(Mac Button)";
  }

  std::string OnClick() const override
  {
    return "Mac button clicked";
  }
};

class MacCheckbox : public Checkbox
{
 public:
  std::string Render() const override
  {
    return "(Mac Checkbox)";
  }
};

class MacTextBox : public TextBox
{
 public:
  std::string Render() const override
  {
    return "(Mac TextBox)";
  }
};

// ---------------------------------------------------------------------------
// Abstract Factory
// ---------------------------------------------------------------------------

class GuiFactory
{
 public:
  virtual ~GuiFactory() = default;
  virtual std::unique_ptr<Button> CreateButton() const = 0;
  virtual std::unique_ptr<Checkbox> CreateCheckbox() const = 0;
  virtual std::unique_ptr<TextBox> CreateTextBox() const = 0;

  // Convenience: create an entire widget set and return it as variants.
  std::array<AnyWidget, 3> CreateWidgetSet() const
  {
    return {
        AnyWidget{CreateButton()},
        AnyWidget{CreateCheckbox()},
        AnyWidget{CreateTextBox()}};
  }
};

// ---------------------------------------------------------------------------
// Concrete Factories
// ---------------------------------------------------------------------------

class WindowsFactory : public GuiFactory
{
 public:
  std::unique_ptr<Button> CreateButton() const override
  {
    return std::make_unique<WindowsButton>();
  }

  std::unique_ptr<Checkbox> CreateCheckbox() const override
  {
    return std::make_unique<WindowsCheckbox>();
  }

  std::unique_ptr<TextBox> CreateTextBox() const override
  {
    return std::make_unique<WindowsTextBox>();
  }
};

class MacFactory : public GuiFactory
{
 public:
  std::unique_ptr<Button> CreateButton() const override
  {
    return std::make_unique<MacButton>();
  }

  std::unique_ptr<Checkbox> CreateCheckbox() const override
  {
    return std::make_unique<MacCheckbox>();
  }

  std::unique_ptr<TextBox> CreateTextBox() const override
  {
    return std::make_unique<MacTextBox>();
  }
};

// ---------------------------------------------------------------------------
// Factory Registry (inline variable — C++17)
// ---------------------------------------------------------------------------

inline const std::unordered_map<std::string_view,
                                std::unique_ptr<GuiFactory>(*)()>
    kFactoryRegistry = {
        {"windows", []() -> std::unique_ptr<GuiFactory> {
           return std::make_unique<WindowsFactory>();
         }},
        {"mac", []() -> std::unique_ptr<GuiFactory> {
           return std::make_unique<MacFactory>();
         }},
};

// Look up a factory by name; returns std::nullopt when not found.
std::optional<std::unique_ptr<GuiFactory>> GetFactory(
    std::string_view name)
{
  if (auto it = kFactoryRegistry.find(name);
      it != kFactoryRegistry.end())
  {
    return it->second();
  }
  return std::nullopt;
}

// ---------------------------------------------------------------------------
// Helper: print a widget using if constexpr on the variant
// ---------------------------------------------------------------------------

void PrintWidget(const AnyWidget& widget)
{
  std::visit(
      [](const auto& w) {
        using T = std::decay_t<decltype(w)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<Button>>)
        {
          std::cout << "  " << w->Render()
                    << "  ->  " << w->OnClick() << "\n";
        }
        else
        {
          std::cout << "  " << w->Render() << "\n";
        }
      },
      widget);
}

// ---------------------------------------------------------------------------
// Client code — works only with abstractions
// ---------------------------------------------------------------------------

void RenderUi(const GuiFactory& factory)
{
  auto [button, checkbox, textbox] = factory.CreateWidgetSet();

  std::cout << "Rendering UI widgets:\n";
  PrintWidget(button);
  PrintWidget(checkbox);
  PrintWidget(textbox);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main()
{
  std::cout << "=== Abstract Factory Pattern Demo (C++17) ===\n\n";

  // Iterate over all registered factories using structured bindings.
  for (const auto& [name, creator] : kFactoryRegistry)
  {
    std::cout << "--- Theme: " << name << " ---\n";
    auto factory = creator();
    RenderUi(*factory);
    std::cout << "\n";
  }

  // Demonstrate std::optional lookup.
  std::cout << "--- Lookup by name ---\n";
  for (auto name : {"windows", "mac", "linux"})
  {
    if (auto factory = GetFactory(name))
    {
      std::cout << "Found factory for \"" << name << "\"\n";
      RenderUi(**factory);
    }
    else
    {
      std::cout << "No factory registered for \"" << name << "\"\n";
    }
    std::cout << "\n";
  }

  return 0;
}
