/**
 * Command Pattern — C++17 Implementation
 *
 * Intent: Encapsulate a request as an object, thereby letting you parameterize
 * clients with different requests, queue or log requests, and support undoable
 * operations.
 *
 * C++17 features used:
 *   - std::unique_ptr for command ownership (RAII)
 *   - std::optional for saving/restoring receiver state on undo
 *   - std::string_view for zero-copy command descriptions
 *   - [[nodiscard]] to prevent ignoring important return values
 *   - inline variables for header-safe constants
 *   - Structured bindings for concise result unpacking
 *
 * Example: A smart-home remote control that can turn lights on/off, set
 * thermostat temperatures, execute macro commands, and undo/redo operations.
 */

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Receivers — the devices that actually perform work
// ---------------------------------------------------------------------------

/// A dimmable smart light.
class Light
{
 public:
  explicit Light(std::string_view location) : location_(location) {}

  void On()
  {
    level_ = k_default_level;
    std::cout << "  " << location_ << " light is ON (level "
              << level_ << "%)\n";
  }

  void Off()
  {
    level_ = 0;
    std::cout << "  " << location_ << " light is OFF\n";
  }

  void SetLevel(int level)
  {
    level_ = std::clamp(level, 0, 100);
    std::cout << "  " << location_ << " light level set to "
              << level_ << "%\n";
  }

  [[nodiscard]] int GetLevel() const { return level_; }
  [[nodiscard]] std::string_view GetLocation() const { return location_; }

 private:
  static inline constexpr int k_default_level = 80;

  std::string location_;
  int level_ = 0;
};

/// A smart thermostat.
class Thermostat
{
 public:
  explicit Thermostat(std::string_view room) : room_(room) {}

  void SetTemperature(double temp)
  {
    temperature_ = temp;
    std::cout << "  " << room_ << " thermostat set to "
              << std::fixed << std::setprecision(1) << temperature_
              << " °C\n";
  }

  [[nodiscard]] double GetTemperature() const { return temperature_; }
  [[nodiscard]] std::string_view GetRoom() const { return room_; }

 private:
  static inline constexpr double k_default_temp = 22.0;

  std::string room_;
  double temperature_ = k_default_temp;
};

// ---------------------------------------------------------------------------
// Command interface
// ---------------------------------------------------------------------------

/// Abstract command: every command can execute and undo.
class Command
{
 public:
  virtual ~Command() = default;

  virtual void Execute() = 0;
  virtual void Undo() = 0;

  [[nodiscard]] virtual std::string_view Description() const = 0;
};

// ---------------------------------------------------------------------------
// Concrete commands — Light
// ---------------------------------------------------------------------------

/// Turn a light on, saving its previous level for undo.
class LightOnCommand final : public Command
{
 public:
  explicit LightOnCommand(Light& light) : light_(light) {}

  void Execute() override
  {
    prev_level_ = light_.GetLevel();
    light_.On();
  }

  void Undo() override
  {
    if (prev_level_.has_value())
    {
      if (*prev_level_ == 0)
      {
        light_.Off();
      }
      else
      {
        light_.SetLevel(*prev_level_);
      }
    }
  }

  [[nodiscard]] std::string_view Description() const override
  {
    return "LightOn";
  }

 private:
  Light& light_;
  std::optional<int> prev_level_;
};

/// Turn a light off, saving its previous level for undo.
class LightOffCommand final : public Command
{
 public:
  explicit LightOffCommand(Light& light) : light_(light) {}

  void Execute() override
  {
    prev_level_ = light_.GetLevel();
    light_.Off();
  }

  void Undo() override
  {
    if (prev_level_.has_value() && *prev_level_ > 0)
    {
      light_.SetLevel(*prev_level_);
    }
  }

  [[nodiscard]] std::string_view Description() const override
  {
    return "LightOff";
  }

 private:
  Light& light_;
  std::optional<int> prev_level_;
};

// ---------------------------------------------------------------------------
// Concrete commands — Thermostat
// ---------------------------------------------------------------------------

/// Set the thermostat to a specific temperature.
class ThermostatSetCommand final : public Command
{
 public:
  ThermostatSetCommand(Thermostat& thermostat, double target_temp)
      : thermostat_(thermostat), target_temp_(target_temp) {}

  void Execute() override
  {
    prev_temp_ = thermostat_.GetTemperature();
    thermostat_.SetTemperature(target_temp_);
  }

  void Undo() override
  {
    if (prev_temp_.has_value())
    {
      thermostat_.SetTemperature(*prev_temp_);
    }
  }

  [[nodiscard]] std::string_view Description() const override
  {
    return "ThermostatSet";
  }

 private:
  Thermostat& thermostat_;
  double target_temp_;
  std::optional<double> prev_temp_;
};

// ---------------------------------------------------------------------------
// Macro command — composes multiple commands into one
// ---------------------------------------------------------------------------

/// A composite command that executes (and undoes) a sequence of commands.
class MacroCommand final : public Command
{
 public:
  explicit MacroCommand(std::string_view name) : name_(name) {}

  void Add(std::unique_ptr<Command> cmd)
  {
    commands_.push_back(std::move(cmd));
  }

  void Execute() override
  {
    std::cout << ">> Executing macro \"" << name_ << "\" ("
              << commands_.size() << " commands)\n";
    for (auto& cmd : commands_)
    {
      cmd->Execute();
    }
  }

  void Undo() override
  {
    std::cout << ">> Undoing macro \"" << name_ << "\"\n";
    // Undo in reverse order.
    for (auto it = commands_.rbegin(); it != commands_.rend(); ++it)
    {
      (*it)->Undo();
    }
  }

  [[nodiscard]] std::string_view Description() const override
  {
    return name_;
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<Command>> commands_;
};

// ---------------------------------------------------------------------------
// Invoker — the remote control with undo/redo history
// ---------------------------------------------------------------------------

/// The remote control that invokes commands and tracks history for undo/redo.
class RemoteControl
{
 public:
  /// Execute a command and push it onto the undo stack.
  void ExecuteCommand(std::unique_ptr<Command> cmd)
  {
    std::cout << "[Remote] Execute: " << cmd->Description() << "\n";
    cmd->Execute();

    // When we execute a new command, any redo history is discarded.
    redo_stack_.clear();
    undo_stack_.push_back(std::move(cmd));
  }

  /// Undo the last executed command.
  [[nodiscard]] bool Undo()
  {
    if (undo_stack_.empty())
    {
      std::cout << "[Remote] Nothing to undo.\n";
      return false;
    }

    auto cmd = std::move(undo_stack_.back());
    undo_stack_.pop_back();

    std::cout << "[Remote] Undo: " << cmd->Description() << "\n";
    cmd->Undo();

    redo_stack_.push_back(std::move(cmd));
    return true;
  }

  /// Redo the last undone command.
  [[nodiscard]] bool Redo()
  {
    if (redo_stack_.empty())
    {
      std::cout << "[Remote] Nothing to redo.\n";
      return false;
    }

    auto cmd = std::move(redo_stack_.back());
    redo_stack_.pop_back();

    std::cout << "[Remote] Redo: " << cmd->Description() << "\n";
    cmd->Execute();

    undo_stack_.push_back(std::move(cmd));
    return true;
  }

  /// Print the current command history.
  void ShowHistory() const
  {
    std::cout << "[Remote] Command history (" << undo_stack_.size()
              << " commands):\n";
    for (std::size_t i = 0; i < undo_stack_.size(); ++i)
    {
      std::cout << "  " << (i + 1) << ". "
                << undo_stack_[i]->Description() << "\n";
    }
  }

 private:
  std::vector<std::unique_ptr<Command>> undo_stack_;
  std::vector<std::unique_ptr<Command>> redo_stack_;
};

// ---------------------------------------------------------------------------
// Helper to create commands with type deduction
// ---------------------------------------------------------------------------

/// Convenience factory using CTAD-like pattern.
template <typename CommandT, typename... Args>
[[nodiscard]] std::unique_ptr<Command> MakeCommand(Args&&... args)
{
  return std::make_unique<CommandT>(std::forward<Args>(args)...);
}

// ---------------------------------------------------------------------------
// Client code — demonstrates the pattern
// ---------------------------------------------------------------------------

int main()
{
  // Create receivers.
  Light living_room_light{"LivingRoom"};
  Light bedroom_light{"Bedroom"};
  Thermostat living_room_thermo{"LivingRoom"};

  // Create the invoker.
  RemoteControl remote;

  std::cout << "=== Command Pattern: Smart Home Remote ===\n\n";

  // --- Individual commands ---
  std::cout << "--- Step 1: Turn on living room light ---\n";
  remote.ExecuteCommand(
      MakeCommand<LightOnCommand>(living_room_light));

  std::cout << "\n--- Step 2: Set thermostat to 24°C ---\n";
  remote.ExecuteCommand(
      MakeCommand<ThermostatSetCommand>(living_room_thermo, 24.0));

  std::cout << "\n--- Step 3: Turn on bedroom light ---\n";
  remote.ExecuteCommand(
      MakeCommand<LightOnCommand>(bedroom_light));

  std::cout << "\n--- Step 4: Turn off living room light ---\n";
  remote.ExecuteCommand(
      MakeCommand<LightOffCommand>(living_room_light));

  // --- Show history ---
  std::cout << "\n";
  remote.ShowHistory();

  // --- Undo sequence ---
  std::cout << "\n--- Undo last 3 commands ---\n";
  (void)remote.Undo();  // Undo: living room light off
  std::cout << "\n";
  (void)remote.Undo();  // Undo: bedroom light on
  std::cout << "\n";
  (void)remote.Undo();  // Undo: thermostat set

  // --- Redo one ---
  std::cout << "\n--- Redo one command ---\n";
  (void)remote.Redo();  // Redo: thermostat set

  // --- Macro command: "Good Night" ---
  std::cout << "\n--- Macro: \"Good Night\" scene ---\n";
  MacroCommand good_night{"GoodNight"};
  good_night.Add(MakeCommand<LightOffCommand>(living_room_light));
  good_night.Add(MakeCommand<LightOffCommand>(bedroom_light));
  good_night.Add(MakeCommand<ThermostatSetCommand>(
      living_room_thermo, 18.0));

  // Execute the macro as a single command (move ownership).
  auto macro_ptr = std::make_unique<MacroCommand>(std::move(good_night));
  remote.ExecuteCommand(std::move(macro_ptr));

  std::cout << "\n";
  remote.ShowHistory();

  // --- Undo the entire macro in one step ---
  std::cout << "\n--- Undo macro (restores all 3 commands at once) ---\n";
  (void)remote.Undo();

  // --- Verify state ---
  std::cout << "\n--- Final device states ---\n";
  const auto& [loc1, lvl1] = std::make_tuple(
      living_room_light.GetLocation(),
      living_room_light.GetLevel());
  const auto& [loc2, lvl2] = std::make_tuple(
      bedroom_light.GetLocation(),
      bedroom_light.GetLevel());
  std::cout << "  " << loc1 << " light level: " << lvl1 << "%\n";
  std::cout << "  " << loc2 << " light level: " << lvl2 << "%\n";
  std::cout << "  " << living_room_thermo.GetRoom() << " thermostat: "
            << std::fixed << std::setprecision(1)
            << living_room_thermo.GetTemperature() << " °C\n";

  return 0;
}
