/**
 * Visitor Pattern — C++17 Implementation
 *
 * Intent:
 *   Represent an operation to be performed on elements of an object
 *   structure. Visitor lets you define a new operation without changing
 *   the classes of the elements on which it operates.
 *
 * C++17 features used:
 *   - std::variant as a type-safe union of document element types
 *   - std::visit with overloaded lambdas for double dispatch
 *   - if constexpr for compile-time type branching
 *   - fold expressions to build the overloaded lambda combinator
 *   - structured bindings for clean container decomposition
 *   - std::optional for nullable element lookup
 *   - inline variables for global constants
 *   - std::string_view for lightweight constant strings
 */

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ===========================================================================
// Helper: Overloaded lambda combinator (fold expression + C++17)
// ===========================================================================

template <typename... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};

// C++17 CTAD (Class Template Argument Deduction) — no need for make helper.
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

// ===========================================================================
// Variant-based Document Elements (modern approach)
// ===========================================================================

struct Paragraph
{
  std::string text;
};

struct Image
{
  std::string url;
  std::string alt;
};

struct Table
{
  std::vector<std::vector<std::string>> rows;
};

struct Heading
{
  std::string text;
  int level;
};

using DocumentElement = std::variant<Paragraph, Image, Table, Heading>;
using Document = std::vector<DocumentElement>;

// ===========================================================================
// Visitor 1: HTML Export (std::visit + overloaded lambda)
// ===========================================================================

class HtmlExportVisitor
{
 public:
  void Visit(const Document& doc)
  {
    for (const auto& element : doc)
    {
      std::visit(*this, element);
    }
  }

  void operator()(const Paragraph& p)
  {
    result_ += "<p>" + p.text + "</p>\n";
  }

  void operator()(const Image& img)
  {
    result_ += "<img src=\"" + img.url + "\" alt=\"" + img.alt + "\" />\n";
  }

  void operator()(const Table& t)
  {
    result_ += "<table>\n";
    for (const auto& row : t.rows)
    {
      result_ += "  <tr>";
      for (const auto& cell : row)
      {
        result_ += "<td>" + cell + "</td>";
      }
      result_ += "</tr>\n";
    }
    result_ += "</table>\n";
  }

  void operator()(const Heading& h)
  {
    result_ += "<h" + std::to_string(h.level) + ">"
             + h.text
             + "</h" + std::to_string(h.level) + ">\n";
  }

  const std::string& GetResult() const { return result_; }

 private:
  std::string result_;
};

// ===========================================================================
// Visitor 2: Markdown Export
// ===========================================================================

class MarkdownExportVisitor
{
 public:
  void Visit(const Document& doc)
  {
    for (const auto& element : doc)
    {
      std::visit(*this, element);
    }
  }

  void operator()(const Paragraph& p)
  {
    result_ += p.text + "\n\n";
  }

  void operator()(const Image& img)
  {
    result_ += "![" + img.alt + "](" + img.url + ")\n\n";
  }

  void operator()(const Table& t)
  {
    if (t.rows.empty())
    {
      return;
    }

    // Header row.
    result_ += "| ";
    for (const auto& cell : t.rows.front())
    {
      result_ += cell + " | ";
    }
    result_ += "\n| ";
    for (size_t i = 0; i < t.rows.front().size(); ++i)
    {
      result_ += "--- | ";
    }
    result_ += "\n";

    // Data rows.
    for (size_t r = 1; r < t.rows.size(); ++r)
    {
      result_ += "| ";
      for (const auto& cell : t.rows[r])
      {
        result_ += cell + " | ";
      }
      result_ += "\n";
    }
    result_ += "\n";
  }

  void operator()(const Heading& h)
  {
    result_ += std::string(h.level, '#') + " " + h.text + "\n\n";
  }

  const std::string& GetResult() const { return result_; }

 private:
  std::string result_;
};

// ===========================================================================
// Visitor 3: Word Count (accumulating state across the traversal)
// ===========================================================================

class WordCountVisitor
{
 public:
  void Visit(const Document& doc)
  {
    for (const auto& element : doc)
    {
      std::visit(*this, element);
    }
  }

  void operator()(const Paragraph& p)
  {
    count_ += CountWords(p.text);
  }

  void operator()(const Image&)
  {
    // Images contribute zero words to the textual count.
  }

  void operator()(const Table& t)
  {
    for (const auto& row : t.rows)
    {
      for (const auto& cell : row)
      {
        count_ += CountWords(cell);
      }
    }
  }

  void operator()(const Heading& h)
  {
    count_ += CountWords(h.text);
  }

  int GetCount() const { return count_; }

 private:
  static int CountWords(std::string_view text)
  {
    int count = 0;
    bool in_word = false;
    for (char ch : text)
    {
      if (ch == ' ' || ch == '\t' || ch == '\n')
      {
        in_word = false;
      }
      else if (!in_word)
      {
        in_word = true;
        ++count;
      }
    }
    return count;
  }

  int count_ = 0;
};

// ===========================================================================
// Variant-based "Accept" — a free function replacing the classic Accept()
// ===========================================================================

// In the classic Visitor pattern, each element has an Accept(Visitor&)
// method that calls visitor.Visit*(*this). With std::variant, this
// dispatching is handled entirely by std::visit, so no Accept() method
// is needed on the element types. This is a key advantage of the
// variant-based approach.

template <typename Visitor, typename Element>
void Accept(Visitor& visitor, const Element& element)
{
  std::visit(visitor, element);
}

// ===========================================================================
// Traditional Visitor (classic double-dispatch via virtual Accept/Visit)
// This demonstrates the textbook approach for comparison.
// ===========================================================================

// Forward declarations.
class ClassicParagraph;
class ClassicImage;
class ClassicTable;
class ClassicHeading;

class ClassicVisitor
{
 public:
  virtual ~ClassicVisitor() = default;
  virtual void VisitParagraph(const ClassicParagraph& e) = 0;
  virtual void VisitImage(const ClassicImage& e) = 0;
  virtual void VisitTable(const ClassicTable& e) = 0;
  virtual void VisitHeading(const ClassicHeading& e) = 0;
};

class ClassicElement
{
 public:
  virtual ~ClassicElement() = default;
  virtual void Accept(ClassicVisitor& visitor) const = 0;
};

class ClassicParagraph : public ClassicElement
{
 public:
  explicit ClassicParagraph(std::string text) : text_(std::move(text)) {}

  void Accept(ClassicVisitor& visitor) const override
  {
    visitor.VisitParagraph(*this);
  }

  const std::string& GetText() const { return text_; }

 private:
  std::string text_;
};

class ClassicImage : public ClassicElement
{
 public:
  ClassicImage(std::string url, std::string alt)
      : url_(std::move(url)), alt_(std::move(alt)) {}

  void Accept(ClassicVisitor& visitor) const override
  {
    visitor.VisitImage(*this);
  }

  const std::string& GetUrl() const { return url_; }
  const std::string& GetAlt() const { return alt_; }

 private:
  std::string url_;
  std::string alt_;
};

class ClassicTable : public ClassicElement
{
 public:
  explicit ClassicTable(std::vector<std::vector<std::string>> rows)
      : rows_(std::move(rows)) {}

  void Accept(ClassicVisitor& visitor) const override
  {
    visitor.VisitTable(*this);
  }

  const std::vector<std::vector<std::string>>& GetRows() const
  {
    return rows_;
  }

 private:
  std::vector<std::vector<std::string>> rows_;
};

class ClassicHeading : public ClassicElement
{
 public:
  ClassicHeading(std::string text, int level)
      : text_(std::move(text)), level_(level) {}

  void Accept(ClassicVisitor& visitor) const override
  {
    visitor.VisitHeading(*this);
  }

  const std::string& GetText() const { return text_; }
  int GetLevel() const { return level_; }

 private:
  std::string text_;
  int level_;
};

// A concrete classic visitor: simple text dump.
class TextDumpVisitor : public ClassicVisitor
{
 public:
  void VisitParagraph(const ClassicParagraph& e) override
  {
    result_ += "[Paragraph] " + e.GetText() + "\n";
  }

  void VisitImage(const ClassicImage& e) override
  {
    result_ += "[Image] " + e.GetUrl() + " (" + e.GetAlt() + ")\n";
  }

  void VisitTable(const ClassicTable& e) override
  {
    result_ += "[Table] " + std::to_string(e.GetRows().size()) + " rows\n";
  }

  void VisitHeading(const ClassicHeading& e) override
  {
    result_ += "[Heading h" + std::to_string(e.GetLevel()) + "] "
             + e.GetText() + "\n";
  }

  const std::string& GetResult() const { return result_; }

 private:
  std::string result_;
};

// ===========================================================================
// Helper: build a sample document
// ===========================================================================

Document BuildSampleDocument()
{
  return Document{
      Heading{"Visitor Pattern", 1},
      Paragraph{
          "The Visitor pattern lets you separate algorithms from the "
          "object structures on which they operate."},
      Image{"visitor_uml.png", "Visitor UML diagram"},
      Heading{"Implementation", 2},
      Paragraph{
          "In C++17 we can use std::variant and std::visit as a modern "
          "alternative to the classic double dispatch mechanism."},
      Table{{{"Feature", "Classic", "Variant"},
             {"Type safety", "Runtime", "Compile-time"},
             {"Open for new operations", "Yes", "Yes"},
             {"Open for new element types", "Yes", "Hard"}}},
  };
}

// ===========================================================================
// Main — demonstrate both variant-based and classic visitors
// ===========================================================================

int main()
{
  std::cout << "=== Visitor Pattern Demo (C++17) ===\n\n";

  auto doc = BuildSampleDocument();

  // -----------------------------------------------------------------------
  // 1. Variant-based visitors (modern C++17 approach)
  // -----------------------------------------------------------------------
  std::cout << "--- Variant-based Visitor: HTML Export ---\n";
  HtmlExportVisitor html_visitor;
  html_visitor.Visit(doc);
  std::cout << html_visitor.GetResult() << "\n";

  std::cout << "--- Variant-based Visitor: Markdown Export ---\n";
  MarkdownExportVisitor md_visitor;
  md_visitor.Visit(doc);
  std::cout << md_visitor.GetResult() << "\n";

  std::cout << "--- Variant-based Visitor: Word Count ---\n";
  WordCountVisitor wc_visitor;
  wc_visitor.Visit(doc);
  std::cout << "Total words: " << wc_visitor.GetCount() << "\n\n";

  // -----------------------------------------------------------------------
  // 2. Demonstrate std::visit with inline overloaded lambda (no class needed)
  // -----------------------------------------------------------------------
  std::cout << "--- Inline Visitor (overloaded lambda) ---\n";
  for (const auto& element : doc)
  {
    std::visit(
        Overloaded{
            [](const Heading& h)
            {
              std::cout << "  [Heading h" << h.level << "] " << h.text << "\n";
            },
            [](const Paragraph& p)
            {
              std::cout << "  [Paragraph] " << p.text << "\n";
            },
            [](const Image& img)
            {
              std::cout << "  [Image] " << img.url << "\n";
            },
            [](const Table& t)
            {
              std::cout << "  [Table] " << t.rows.size() << " rows\n";
            },
        },
        element);
  }
  std::cout << "\n";

  // -----------------------------------------------------------------------
  // 3. Structured bindings + if constexpr: extract info from elements
  // -----------------------------------------------------------------------
  std::cout << "--- Structured Bindings + if constexpr ---\n";

  // Structured binding to deconstruct the first Heading.
  const auto& first_element = doc.front();
  if (const auto* heading = std::get_if<Heading>(&first_element))
  {
    auto [text, level] = *heading;
    std::cout << "First element is a Heading: \"" << text
              << "\" (level " << level << ")\n";
  }

  // if constexpr: a generic lambda that returns a description string,
  // branching at compile time on the variant's active type.
  auto describe = [](const DocumentElement& elem) -> std::string
  {
    return std::visit(
        [](const auto& e) -> std::string
        {
          using T = std::decay_t<decltype(e)>;
          if constexpr (std::is_same_v<T, Heading>)
          {
            return "Heading[h" + std::to_string(e.level) + "]: " + e.text;
          }
          else if constexpr (std::is_same_v<T, Paragraph>)
          {
            return "Paragraph: " + e.text.substr(0, 40) + "...";
          }
          else if constexpr (std::is_same_v<T, Image>)
          {
            return "Image: " + e.url;
          }
          else if constexpr (std::is_same_v<T, Table>)
          {
            return "Table: " + std::to_string(e.rows.size()) + " rows";
          }
        },
        elem);
  };

  for (size_t i = 0; i < doc.size(); ++i)
  {
    std::cout << "  doc[" << i << "] -> " << describe(doc[i]) << "\n";
  }
  std::cout << "\n";

  // -----------------------------------------------------------------------
  // 4. std::optional: find the first image in the document
  // -----------------------------------------------------------------------
  std::cout << "--- std::optional: Find First Image ---\n";
  auto find_first_image = [](const Document& d)
      -> std::optional<Image>
  {
    for (const auto& elem : d)
    {
      if (auto* img = std::get_if<Image>(&elem))
      {
        return *img;
      }
    }
    return std::nullopt;
  };

  if (auto img = find_first_image(doc))
  {
    std::cout << "Found image: url=\"" << img->url
              << "\", alt=\"" << img->alt << "\"\n";
  }
  else
  {
    std::cout << "No image found.\n";
  }
  std::cout << "\n";

  // -----------------------------------------------------------------------
  // 5. Classic visitor (traditional double-dispatch) for comparison
  // -----------------------------------------------------------------------
  std::cout << "--- Classic Visitor (double dispatch) ---\n";
  std::vector<std::unique_ptr<ClassicElement>> classic_doc;
  classic_doc.push_back(
      std::make_unique<ClassicHeading>("Visitor Pattern", 1));
  classic_doc.push_back(std::make_unique<ClassicParagraph>(
      "The Visitor pattern separates algorithms from object structures."));
  classic_doc.push_back(std::make_unique<ClassicImage>(
      "visitor_uml.png", "Visitor UML diagram"));
  classic_doc.push_back(std::make_unique<ClassicTable>(
      std::vector<std::vector<std::string>>{
          {"Approach", "Pros", "Cons"},
          {"Classic", "Open for new elements", "Verbose"},
          {"Variant", "Concise, type-safe", "Hard to add elements"}}));

  TextDumpVisitor dump_visitor;
  for (const auto& elem : classic_doc)
  {
    elem->Accept(dump_visitor);
  }
  std::cout << dump_visitor.GetResult() << "\n";

  // -----------------------------------------------------------------------
  // 6. Summary comparison
  // -----------------------------------------------------------------------
  std::cout << "--- Summary ---\n";
  std::cout << "Variant-based (modern C++17):\n"
            << "  - No virtual functions needed for elements\n"
            << "  - std::visit dispatches by type at compile time\n"
            << "  - Overloaded lambdas replace Visitor interface\n"
            << "  - Adding new element types requires changing the variant\n"
            << "    definition and all visitor lambdas/classes\n\n"
            << "Classic double-dispatch:\n"
            << "  - Each element has a virtual Accept() method\n"
            << "  - Each visitor declares visit methods for all element types\n"
            << "  - Adding new operations requires a new Visitor subclass\n"
            << "  - Adding new element types requires modifying all visitors\n";

  return 0;
}
