/**
 * Flyweight Pattern - C++17 Implementation
 *
 * Intent: Use sharing to support large numbers of fine-grained objects
 * efficiently by separating intrinsic (shared) state from extrinsic
 * (unique) state.
 *
 * C++17 features used:
 *   - std::optional for optional font properties
 *   - std::variant for type-safe glyph rendering styles
 *   - Structured bindings for clean map iteration
 *   - if constexpr for compile-time rendering dispatch
 *   - std::shared_mutex for thread-safe factory
 *   - inline variables for shared constants
 *   - Class template argument deduction (CTAD)
 */

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>
#include <shared_mutex>
#include <mutex>
#include <sstream>
#include <functional>
#include <iomanip>

// ============================================================================
// Constants
// ============================================================================

inline constexpr int kMaxFontSize = 128;
inline constexpr int kDefaultFontSize = 12;
inline constexpr char kDefaultFont[] = "Arial";

// ============================================================================
// Intrinsic State - Shared across many objects (Flyweight)
// ============================================================================

/**
 * FontStyle represents the intrinsic (shared) state of a character.
 * Multiple characters with the same font style share this single object.
 */
struct FontStyle
{
  std::string font_family;
  int font_size;
  bool is_bold;
  bool is_italic;

  bool operator==(const FontStyle& other) const
  {
    return font_family == other.font_family &&
           font_size == other.font_size &&
           is_bold == other.is_bold &&
           is_italic == other.is_italic;
  }
};

/**
 * Hash function for FontStyle to use in unordered_map.
 */
struct FontStyleHash
{
  std::size_t operator()(const FontStyle& style) const
  {
    std::size_t h1 = std::hash<std::string>{}(style.font_family);
    std::size_t h2 = std::hash<int>{}(style.font_size);
    std::size_t h3 = std::hash<bool>{}(style.is_bold);
    std::size_t h4 = std::hash<bool>{}(style.is_italic);
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
  }
};

// ============================================================================
// Rendering Style - Variant for different rendering modes
// ============================================================================

struct NormalRender {};
struct HighlightRender { std::string highlight_color; };
struct UnderlineRender {};
struct StrikethroughRender {};

using RenderStyle = std::variant<
  NormalRender,
  HighlightRender,
  UnderlineRender,
  StrikethroughRender
>;

// ============================================================================
// Glyph - Abstract Flyweight Interface
// ============================================================================

/**
 * Glyph is the abstract flyweight interface.
 * Defines operations that accept extrinsic state as parameters.
 */
class Glyph
{
 public:
  virtual ~Glyph() = default;

  /**
   * Render the glyph at the given position with optional render style.
   * Position and style are extrinsic state passed by the client.
   */
  virtual void Render(int row, int col,
                      const RenderStyle& style = NormalRender{}) const = 0;

  /**
   * Get the intrinsic font style of this glyph.
   */
  virtual const FontStyle& GetFontStyle() const = 0;

  /**
   * Get the character this glyph represents.
   */
  virtual char GetCharacter() const = 0;
};

// ============================================================================
// CharacterGlyph - Concrete Flyweight
// ============================================================================

/**
 * CharacterGlyph is a concrete flyweight that stores intrinsic state:
 * the character value and its font style. This state is shared among
 * all positions where this character appears with the same style.
 */
class CharacterGlyph : public Glyph
{
 public:
  CharacterGlyph(char ch, FontStyle style)
      : character_(ch), style_(std::move(style))
  {
  }

  void Render(int row, int col,
              const RenderStyle& style) const override
  {
    // Apply rendering style using if constexpr via visitor
    std::visit([&](const auto& render_mode)
    {
      using T = std::decay_t<decltype(render_mode)>;

      if constexpr (std::is_same_v<T, NormalRender>)
      {
        std::cout << "  [" << row << "," << col << "] '"
                  << character_ << "' " << FormatStyle();
      }
      else if constexpr (std::is_same_v<T, HighlightRender>)
      {
        std::cout << "  [" << row << "," << col << "] '"
                  << character_ << "' " << FormatStyle()
                  << " [HIGHLIGHT:" << render_mode.highlight_color << "]";
      }
      else if constexpr (std::is_same_v<T, UnderlineRender>)
      {
        std::cout << "  [" << row << "," << col << "] '"
                  << character_ << "' " << FormatStyle()
                  << " [UNDERLINE]";
      }
      else if constexpr (std::is_same_v<T, StrikethroughRender>)
      {
        std::cout << "  [" << row << "," << col << "] '"
                  << character_ << "' " << FormatStyle()
                  << " [STRIKETHROUGH]";
      }
    }, style);

    std::cout << "\n";
  }

  const FontStyle& GetFontStyle() const override
  {
    return style_;
  }

  char GetCharacter() const override
  {
    return character_;
  }

 private:
  std::string FormatStyle() const
  {
    std::ostringstream oss;
    oss << "(" << style_.font_family << ", " << style_.font_size << "pt";
    if (style_.is_bold) oss << ", Bold";
    if (style_.is_italic) oss << ", Italic";
    oss << ")";
    return oss.str();
  }

  char character_;
  FontStyle style_;
};

// ============================================================================
// FlyweightFactory - Creates and manages flyweight objects
// ============================================================================

/**
 * FlyweightFactory manages the pool of shared flyweight objects.
 * Uses a hash map to ensure that identical intrinsic states share
 * the same flyweight instance.
 *
 * Thread-safe via std::shared_mutex for concurrent access.
 */
class FlyweightFactory
{
 public:
  /**
   * Get or create a flyweight for the given character and style.
   * Returns a shared_ptr to the existing flyweight if one with
   * matching intrinsic state already exists.
   */
  std::shared_ptr<Glyph> GetGlyph(char ch, const FontStyle& style)
  {
    GlyphKey key{ch, style};

    // First try read lock for fast path
    {
      std::shared_lock<std::shared_mutex> read_lock(mutex_);
      auto it = flyweights_.find(key);
      if (it != flyweights_.end())
      {
        return it->second;
      }
    }

    // Acquire write lock to insert new flyweight
    {
      std::unique_lock<std::shared_mutex> write_lock(mutex_);
      // Double-check after acquiring write lock
      auto it = flyweights_.find(key);
      if (it != flyweights_.end())
      {
        return it->second;
      }

      auto glyph = std::make_shared<CharacterGlyph>(ch, style);
      flyweights_.emplace(key, glyph);
      return glyph;
    }
  }

  /**
   * Get the total number of flyweight instances in the pool.
   */
  std::size_t GetPoolSize() const
  {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return flyweights_.size();
  }

  /**
   * Print statistics about the flyweight pool.
   */
  void PrintStats() const
  {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    std::cout << "\n=== Flyweight Pool Statistics ===\n";
    std::cout << "Unique flyweight instances: " << flyweights_.size() << "\n";
    std::cout << "Pooled combinations:\n";
    for (const auto& [key, glyph] : flyweights_)
    {
      std::cout << "  '" << key.character << "' -> "
                << glyph->GetFontStyle().font_family << " "
                << glyph->GetFontStyle().font_size << "pt";
      if (glyph->GetFontStyle().is_bold) std::cout << " Bold";
      if (glyph->GetFontStyle().is_italic) std::cout << " Italic";
      std::cout << "\n";
    }
  }

 private:
  struct GlyphKey
  {
    char character;
    FontStyle style;

    bool operator==(const GlyphKey& other) const
    {
      return character == other.character && style == other.style;
    }
  };

  struct GlyphKeyHash
  {
    std::size_t operator()(const GlyphKey& key) const
    {
      std::size_t h1 = std::hash<char>{}(key.character);
      std::size_t h2 = FontStyleHash{}(key.style);
      return h1 ^ (h2 << 1);
    }
  };

  mutable std::shared_mutex mutex_;
  std::unordered_map<GlyphKey, std::shared_ptr<Glyph>, GlyphKeyHash> flyweights_;
};

// ============================================================================
// DocumentCharacter - Context object holding extrinsic state
// ============================================================================

/**
 * DocumentCharacter represents a character in a document.
 * It holds extrinsic state (position, render style) and a reference
 * to the shared flyweight (glyph) that holds intrinsic state.
 */
struct DocumentCharacter
{
  int row;
  int col;
  RenderStyle render_style;
  std::shared_ptr<Glyph> glyph;

  void Display() const
  {
    glyph->Render(row, col, render_style);
  }
};

// ============================================================================
// Document - Client that uses flyweights
// ============================================================================

/**
 * Document is the client that manages a collection of characters.
 * It uses the FlyweightFactory to efficiently share glyph objects.
 */
class Document
{
 public:
  explicit Document(std::shared_ptr<FlyweightFactory> factory)
      : factory_(std::move(factory))
  {
  }

  /**
   * Add a character to the document at the given position.
   * The factory ensures that identical characters with the same
   * font style share a single flyweight instance.
   */
  void AddCharacter(char ch, int row, int col,
                    const FontStyle& style,
                    const RenderStyle& render_style = NormalRender{})
  {
    auto glyph = factory_->GetGlyph(ch, style);
    characters_.push_back({row, col, render_style, glyph});
  }

  /**
   * Render the entire document.
   */
  void Render() const
  {
    std::cout << "\n=== Document Render ===\n";
    for (const auto& ch : characters_)
    {
      ch.Display();
    }
  }

  /**
   * Get the total number of characters in the document.
   */
  std::size_t GetCharacterCount() const
  {
    return characters_.size();
  }

 private:
  std::shared_ptr<FlyweightFactory> factory_;
  std::vector<DocumentCharacter> characters_;
};

// ============================================================================
// Helper functions for creating common font styles
// ============================================================================

FontStyle MakeHeadingStyle()
{
  return {"Helvetica", 24, true, false};
}

FontStyle MakeBodyStyle()
{
  return {kDefaultFont, kDefaultFontSize, false, false};
}

FontStyle MakeCodeStyle()
{
  return {"Courier New", 11, false, false};
}

FontStyle MakeEmphasisStyle()
{
  return {kDefaultFont, kDefaultFontSize, false, true};
}

// ============================================================================
// Main - Demonstration
// ============================================================================

int main()
{
  std::cout << "========================================\n";
  std::cout << "  Flyweight Pattern - C++17 Demo\n";
  std::cout << "========================================\n";

  // Create shared factory
  auto factory = std::make_shared<FlyweightFactory>();

  // Create document
  Document doc(factory);

  // Define styles
  FontStyle heading_style = MakeHeadingStyle();
  FontStyle body_style = MakeBodyStyle();
  FontStyle code_style = MakeCodeStyle();
  FontStyle emphasis_style = MakeEmphasisStyle();

  // Build a document with many characters
  // Notice: many characters share the same font style

  // Heading: "HELLO"
  std::string heading = "HELLO";
  for (std::size_t i = 0; i < heading.size(); ++i)
  {
    doc.AddCharacter(heading[i], 0, static_cast<int>(i),
                     heading_style, HighlightRender{"yellow"});
  }

  // Body text: "Hello World"
  std::string body_text = "Hello World";
  int col = 0;
  for (char ch : body_text)
  {
    doc.AddCharacter(ch, 1, col, body_style);
    ++col;
  }

  // Code snippet: "int x = 42;"
  std::string code_text = "int x = 42;";
  col = 0;
  for (char ch : code_text)
  {
    doc.AddCharacter(ch, 2, col, code_style, UnderlineRender{});
    ++col;
  }

  // Emphasized text: "Important!"
  std::string emph_text = "Important!";
  col = 0;
  for (char ch : emph_text)
  {
    doc.AddCharacter(ch, 3, col, emphasis_style,
                     StrikethroughRender{});
    ++col;
  }

  // Add repeated body text to demonstrate sharing
  // In a real document, the same characters/styles repeat thousands of times
  std::string paragraph =
      "The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. "
      "Pack my box with five dozen liquor jugs. "
      "Pack my box with five dozen liquor jugs.";
  col = 0;
  int row = 5;
  for (char ch : paragraph)
  {
    if (ch == '\n' || col >= 60)
    {
      ++row;
      col = 0;
    }
    doc.AddCharacter(ch, row, col, body_style);
    ++col;
  }

  // Render the document
  doc.Render();

  // Print statistics
  factory->PrintStats();

  // Demonstrate memory savings
  // In a real scenario, each character without flyweight stores the full
  // intrinsic state (font family string, size, bold, italic) plus its
  // position. With flyweight, only the shared intrinsic state is stored
  // once per unique style, and each character stores a lightweight context.
  std::cout << "\n=== Memory Analysis ===\n";
  std::size_t total_chars = doc.GetCharacterCount();
  std::size_t unique_flyweights = factory->GetPoolSize();

  // Without flyweight: each char stores character + full font info + position
  // Simulated as: char + string(~32 bytes) + int + bool + bool + int + int
  constexpr std::size_t kCharWithoutFlyweight = 1 + 32 + 4 + 1 + 1 + 4 + 4;
  std::size_t without_flyweight = total_chars * kCharWithoutFlyweight;

  // With flyweight: unique flyweights hold intrinsic state,
  // each character holds a lightweight context (position + pointer)
  std::size_t with_flyweight = unique_flyweights * sizeof(CharacterGlyph)
                               + total_chars * (sizeof(int) * 2 + sizeof(void*));

  std::cout << "Total characters: " << total_chars << "\n";
  std::cout << "Unique flyweights: " << unique_flyweights << "\n";
  std::cout << "Without flyweight: ~" << without_flyweight << " bytes\n";
  std::cout << "With flyweight:    ~" << with_flyweight << " bytes\n";
  std::cout << "Memory saved:      ~"
            << (without_flyweight > with_flyweight
                ? without_flyweight - with_flyweight : 0)
            << " bytes ("
            << std::fixed << std::setprecision(1)
            << (without_flyweight > 0
                ? (1.0 - static_cast<double>(with_flyweight) / without_flyweight) * 100.0
                : 0.0)
            << "% reduction)\n";

  // Demonstrate structured bindings with factory stats
  std::cout << "\n=== Structured Binding Demo ===\n";
  auto pool_size = factory->GetPoolSize();
  auto char_count = doc.GetCharacterCount();
  auto [unique_count, total_count] = std::make_pair(pool_size, char_count);
  std::cout << "Unique/Total: " << unique_count << "/" << total_count
            << " (ratio: " << std::fixed << std::setprecision(2)
            << (total_count > 0
                ? static_cast<double>(unique_count) / total_count
                : 0.0)
            << ")\n";

  std::cout << "\n========================================\n";
  std::cout << "  Pattern demonstration complete.\n";
  std::cout << "========================================\n";

  return 0;
}
