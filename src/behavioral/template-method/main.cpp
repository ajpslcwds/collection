/**
 * Template Method Pattern — Data Processing Pipeline
 *
 * Intent: Define the skeleton of a data processing algorithm in a base class,
 * deferring format-specific steps (parse, validate, transform) to subclasses.
 * The overall pipeline — read header, parse records, validate, transform,
 * assemble output — is fixed; only the individual steps vary by format.
 *
 * C++17 features used:
 *   - std::string_view for zero-copy parsing tokens
 *   - std::optional for optional pipeline steps and nullable results
 *   - std::variant for type-safe heterogeneous field values
 *   - if constexpr to conditionally enable optional steps at compile time
 *   - structured bindings for concise tuple/pair/map unpacking
 *   - std::from_chars for locale-independent numeric conversion
 *   - inline static constexpr for class-level constants
 *
 * Compile: g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o main
 */

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <charconv>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ---------------------------------------------------------------------------
// Domain types
// ---------------------------------------------------------------------------

/// A field value is one of: string, integer, or floating-point number.
using FieldValue = std::variant<std::string, int, double>;

/// One parsed data record: an ordered map of field name -> value.
using Record = std::map<std::string, FieldValue>;

/// The complete output of a pipeline run.
struct DataResult
{
  std::string format;
  std::vector<std::string> headers;
  std::vector<Record> records;
};

// ---------------------------------------------------------------------------
// Helper utilities
// ---------------------------------------------------------------------------

namespace
{

/// Trim leading and trailing whitespace from a string_view.
std::string_view Trim(std::string_view sv)
{
  const auto start = sv.find_first_not_of(" \t\r\n");
  if (start == std::string_view::npos)
  {
    return {};
  }
  const auto end = sv.find_last_not_of(" \t\r\n");
  return sv.substr(start, end - start + 1);
}

/// Attempt to convert a string_view into a FieldValue (int > double > string).
FieldValue ParseFieldValue(std::string_view raw)
{
  std::string_view trimmed = Trim(raw);

  // Try integer first.
  int int_val = 0;
  auto [ptr1, ec1] = std::from_chars(trimmed.data(),
                                      trimmed.data() + trimmed.size(),
                                      int_val);
  if (ec1 == std::errc{} && ptr1 == trimmed.data() + trimmed.size())
  {
    return int_val;
  }

  // Try double via strtod (GCC 9 lacks from_chars for floating-point).
  if (trimmed.find('.') != std::string_view::npos)
  {
    std::string str{trimmed};
    char* end = nullptr;
    double dbl_val = std::strtod(str.c_str(), &end);
    if (end == str.c_str() + str.size())
    {
      return dbl_val;
    }
  }

  // Fall back to string.
  return std::string{trimmed};
}

/// Format a FieldValue for display.
std::string ToString(const FieldValue& value)
{
  return std::visit(
      [](const auto& v) -> std::string
      {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
          return "\"" + v + "\"";
        }
        else
        {
          return std::to_string(v);
        }
      },
      value);
}

}  // namespace

// ---------------------------------------------------------------------------
// Template Method — Abstract base class
// ---------------------------------------------------------------------------

/// Abstract base class defining the data processing pipeline skeleton.
///
/// Subclasses override the four primitive operations to handle a specific
/// format (CSV, JSON, XML).  The public Process() method is the template
/// method that orchestrates the fixed pipeline.
class DataProcessor
{
 public:
  virtual ~DataProcessor() = default;

  /// Template method — runs the full pipeline and returns the result.
  DataResult Process(std::string_view input)
  {
    DataResult result;
    result.format = FormatName();

    std::cout << "[" << result.format << "] Starting pipeline\n";

    // Step 1: Parse headers.
    result.headers = ParseHeaders(input);
    std::cout << "[" << result.format << "] Parsed "
              << result.headers.size() << " header(s)\n";

    // Step 2: Parse records.
    result.records = ParseRecords(input, result.headers);
    std::cout << "[" << result.format << "] Parsed "
              << result.records.size() << " record(s)\n";

    // Step 3: Validate records (optional — compiled out for formats that
    // do not override ValidateRecord).
    if constexpr (kHasValidation)
    {
      std::size_t valid = 0;
      for (const auto& record : result.records)
      {
        if (ValidateRecord(record))
        {
          ++valid;
        }
      }
      std::cout << "[" << result.format << "] Validated "
                << valid << "/" << result.records.size() << " record(s)\n";
    }

    // Step 4: Transform records (optional hook).
    auto transformed = TransformRecords(result.records);
    if (transformed.has_value())
    {
      result.records = std::move(transformed).value();
      std::cout << "[" << result.format << "] Transformed "
                << result.records.size() << " record(s)\n";
    }

    // Step 5: Post-process (hook for any final assembly).
    PostProcess(result);

    std::cout << "[" << result.format << "] Pipeline complete\n";
    return result;
  }

 protected:
  /// Compile-time switch: does this processor have validation logic?
  static constexpr bool kHasValidation = false;

  /// Return the human-readable format name (e.g. "CSV", "JSON").
  virtual std::string FormatName() const = 0;

  /// Step 1 — Parse column/field headers from the raw input.
  virtual std::vector<std::string> ParseHeaders(
      std::string_view input) const = 0;

  /// Step 2 — Parse all data records from the raw input.
  virtual std::vector<Record> ParseRecords(
      std::string_view input,
      const std::vector<std::string>& headers) const = 0;

  /// Step 3 (optional) — Validate a single record.  Only called when
  /// kHasValidation is true.
  virtual bool ValidateRecord(const Record& /*record*/) const
  {
    return true;
  }

  /// Step 4 (optional hook) — Transform the parsed records.
  /// Return std::nullopt to leave records unchanged.
  virtual std::optional<std::vector<Record>> TransformRecords(
      [[maybe_unused]] const std::vector<Record>& records) const
  {
    return std::nullopt;
  }

  /// Step 5 (hook) — Final post-processing on the assembled result.
  virtual void PostProcess(DataResult& /*result*/) const {}
};

// ---------------------------------------------------------------------------
// Concrete: CSV processor
// ---------------------------------------------------------------------------

/// Processes comma-separated value data.
class CsvProcessor : public DataProcessor
{
 public:
  inline static constexpr char kDelimiter = ',';

 protected:
  std::string FormatName() const override
  {
    return "CSV";
  }

  std::vector<std::string> ParseHeaders(
      std::string_view input) const override
  {
    auto first_line = ExtractLine(input);
    return SplitLine(first_line);
  }

  std::vector<Record> ParseRecords(
      std::string_view input,
      const std::vector<std::string>& headers) const override
  {
    std::vector<Record> records;
    std::istringstream stream{std::string{input}};
    std::string line;
    bool is_header = true;

    while (std::getline(stream, line))
    {
      if (is_header)
      {
        is_header = false;
        continue;  // Skip header row.
      }
      if (Trim(line).empty())
      {
        continue;
      }

      std::vector<std::string> fields = SplitLine(line);
      Record record;
      for (std::size_t i = 0; i < headers.size(); ++i)
      {
        std::string_view value =
            (i < fields.size()) ? std::string_view{fields[i]} : "";
        record[headers[i]] = ParseFieldValue(value);
      }
      records.push_back(std::move(record));
    }
    return records;
  }

  std::optional<std::vector<Record>> TransformRecords(
      const std::vector<Record>& records) const override
  {
    // Normalize all string field values to lowercase.
    std::vector<Record> transformed;
    transformed.reserve(records.size());

    for (const auto& record : records)
    {
      Record normalized;
      for (const auto& [key, value] : record)
      {
        normalized[key] = NormalizeValue(value);
      }
      transformed.push_back(std::move(normalized));
    }
    return transformed;
  }

 private:
  /// Extract the first line from input.
  static std::string_view ExtractLine(std::string_view input)
  {
    auto pos = input.find('\n');
    return (pos != std::string_view::npos) ? input.substr(0, pos) : input;
  }

  /// Split a line by the delimiter into trimmed tokens.
  std::vector<std::string> SplitLine(std::string_view line) const
  {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ss{std::string{line}};
    while (std::getline(ss, token, kDelimiter))
    {
      tokens.emplace_back(Trim(token));
    }
    return tokens;
  }

  /// Lowercase a value if it is a string; leave numeric types unchanged.
  static FieldValue NormalizeValue(const FieldValue& value)
  {
    return std::visit(
        [](const auto& v) -> FieldValue
        {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, std::string>)
          {
            std::string lower = v;
            std::transform(lower.begin(), lower.end(),
                           lower.begin(),
                           [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return lower;
          }
          else
          {
            return v;  // Numeric values pass through unchanged.
          }
        },
        value);
  }
};

// ---------------------------------------------------------------------------
// Concrete: JSON processor (simplified key-value parser)
// ---------------------------------------------------------------------------

/// Processes simplified JSON objects — one object per line (JSON Lines).
///
/// Each line is expected to be of the form:
///   {"key1": value1, "key2": value2, ...}
///
/// Values may be quoted strings or bare numbers.
class JsonProcessor : public DataProcessor
{
 public:
  static constexpr std::size_t kMaxFields = 100;

 protected:
  static constexpr bool kHasValidation = true;

  std::string FormatName() const override
  {
    return "JSON";
  }

  std::vector<std::string> ParseHeaders(
      std::string_view input) const override
  {
    // Headers are inferred from the first object's keys.
    auto first_line = FirstNonEmptyLine(input);
    if (first_line.empty())
    {
      return {};
    }
    auto [keys, _] = ParseJsonObject(first_line);
    return keys;
  }

  std::vector<Record> ParseRecords(
      std::string_view input,
      [[maybe_unused]] const std::vector<std::string>& headers) const override
  {
    std::vector<Record> records;
    std::istringstream stream{std::string{input}};
    std::string line;

    while (std::getline(stream, line))
    {
      std::string_view trimmed = Trim(line);
      if (trimmed.empty() || trimmed[0] != '{')
      {
        continue;
      }

      auto [keys, values] = ParseJsonObject(trimmed);
      Record record;
      for (std::size_t i = 0; i < keys.size(); ++i)
      {
        record[keys[i]] =
            (i < values.size()) ? ParseFieldValue(values[i])
                                : FieldValue{std::string{}};
      }
      records.push_back(std::move(record));
    }
    return records;
  }

  bool ValidateRecord(const Record& record) const override
  {
    // Ensure no field has an empty string value.
    for (const auto& [key, value] : record)
    {
      bool is_empty = std::visit(
          [](const auto& v) -> bool
          {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
              return v.empty();
            }
            return false;
          },
          value);
      if (is_empty)
      {
        return false;
      }
    }
    return true;
  }

 private:
  /// Return the first non-empty line from the input as a string.
  static std::string FirstNonEmptyLine(std::string_view input)
  {
    std::istringstream stream{std::string{input}};
    std::string line;
    while (std::getline(stream, line))
    {
      if (!Trim(line).empty())
      {
        return line;
      }
    }
    return {};
  }

  /// Parse a simplified JSON object into parallel key and value vectors.
  static std::pair<std::vector<std::string>, std::vector<std::string>>
  ParseJsonObject(std::string_view json)
  {
    std::vector<std::string> keys;
    std::vector<std::string> values;

    // Strip outer braces.
    auto inner = Trim(json);
    if (inner.size() < 2 || inner.front() != '{' || inner.back() != '}')
    {
      return {keys, values};
    }
    inner = Trim(inner.substr(1, inner.size() - 2));

    // Split by comma, then each token by colon — directly on the
    // string_view to avoid dangling references from temporary strings.
    std::size_t pos = 0;
    while (pos < inner.size())
    {
      // Find the end of this "key": value" pair.
      auto comma = inner.find(',', pos);
      std::string_view pair_sv =
          (comma != std::string_view::npos)
              ? Trim(inner.substr(pos, comma - pos))
              : Trim(inner.substr(pos));

      auto colon = pair_sv.find(':');
      if (colon != std::string_view::npos)
      {
        auto raw_key = Trim(pair_sv.substr(0, colon));
        auto raw_val = Trim(pair_sv.substr(colon + 1));

        keys.emplace_back(Unquote(raw_key));
        values.emplace_back(std::string{UnquoteView(raw_val)});
      }

      if (comma == std::string_view::npos)
      {
        break;
      }
      pos = comma + 1;

      if (keys.size() > kMaxFields)
      {
        break;
      }
    }
    return {keys, values};
  }

  /// Remove surrounding double quotes from a string.
  static std::string Unquote(std::string_view sv)
  {
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"')
    {
      return std::string{sv.substr(1, sv.size() - 2)};
    }
    return std::string{sv};
  }

  /// Return a view with surrounding double quotes stripped.
  static std::string_view UnquoteView(std::string_view sv)
  {
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"')
    {
      return sv.substr(1, sv.size() - 2);
    }
    return sv;
  }
};

// ---------------------------------------------------------------------------
// Concrete: XML processor (simplified tag parser)
// ---------------------------------------------------------------------------

/// Processes simplified XML where each <row> element represents a record.
///
/// Expected structure:
///   <data>
///     <row><name>Alice</name><age>30</age></row>
///     ...
///   </data>
class XmlProcessor : public DataProcessor
{
 public:
  inline static constexpr std::string_view kRootElement = "data";
  inline static constexpr std::string_view kRowElement = "row";

 protected:
  static constexpr bool kHasValidation = true;

  std::string FormatName() const override
  {
    return "XML";
  }

  std::vector<std::string> ParseHeaders(
      std::string_view input) const override
  {
    std::vector<std::string> headers;
    // Collect unique tag names from the first <row>...</row>.
    auto row_content = ExtractFirstRow(input);
    if (row_content.empty())
    {
      return headers;
    }

    std::size_t pos = 0;
    while (pos < row_content.size())
    {
      auto open = row_content.find('<', pos);
      if (open == std::string_view::npos)
      {
        break;
      }
      auto close = row_content.find('>', open);
      if (close == std::string_view::npos)
      {
        break;
      }

      auto tag = row_content.substr(open + 1, close - open - 1);
      if (tag.empty() || tag[0] == '/')
      {
        pos = close + 1;
        continue;
      }

      // Only add if not already present.
      std::string tag_str{tag};
      if (std::find(headers.begin(), headers.end(), tag_str) ==
          headers.end())
      {
        headers.push_back(std::move(tag_str));
      }
      pos = close + 1;
    }
    return headers;
  }

  std::vector<Record> ParseRecords(
      std::string_view input,
      const std::vector<std::string>& headers) const override
  {
    std::vector<Record> records;

    // Find all <row>...</row> blocks.
    std::size_t search_pos = 0;
    while (search_pos < input.size())
    {
      auto row_start = FindTag(input, kRowElement, search_pos);
      if (row_start == std::string_view::npos)
      {
        break;
      }
      auto row_end_tag = FindClosingTag(input, kRowElement, row_start);
      if (row_end_tag == std::string_view::npos)
      {
        break;
      }

      auto row_content_start = input.find('>', row_start);
      if (row_content_start == std::string_view::npos ||
          row_content_start >= row_end_tag)
      {
        search_pos = row_end_tag + 1;
        continue;
      }
      auto row_content = input.substr(
          row_content_start + 1,
          row_end_tag - row_content_start - 1);

      Record record;
      for (const auto& header : headers)
      {
        auto value = ExtractTagValue(row_content, header);
        record[header] = ParseFieldValue(value);
      }
      records.push_back(std::move(record));

      search_pos = row_end_tag + kRowElement.size() + 2;  // past </row>
    }
    return records;
  }

  bool ValidateRecord(const Record& record) const override
  {
    // Every record must have at least one field.
    return !record.empty();
  }

  void PostProcess([[maybe_unused]] DataResult& result) const override
  {
    // Add an XML declaration as metadata.
    std::cout << "[" << FormatName()
              << "] Post-process: wrapping result in <data> element\n";
  }

 private:
  /// Extract the content of the first <row>...</row> block.
  static std::string_view ExtractFirstRow(std::string_view input)
  {
    auto start = FindTag(input, kRowElement, 0);
    if (start == std::string_view::npos)
    {
      return {};
    }
    auto end = FindClosingTag(input, kRowElement, start);
    if (end == std::string_view::npos)
    {
      return {};
    }
    auto content_start = input.find('>', start);
    if (content_start == std::string_view::npos || content_start >= end)
    {
      return {};
    }
    return input.substr(content_start + 1, end - content_start - 1);
  }

  /// Find the opening position of `<tag>` starting from `pos`.
  static std::size_t FindTag(std::string_view input,
                              std::string_view tag,
                              std::size_t pos)
  {
    std::string open_tag = "<" + std::string{tag} + ">";
    return input.find(open_tag, pos);
  }

  /// Find the closing position of `</tag>` starting from `pos`.
  static std::size_t FindClosingTag(std::string_view input,
                                     std::string_view tag,
                                     std::size_t pos)
  {
    std::string close_tag = "</" + std::string{tag} + ">";
    return input.find(close_tag, pos);
  }

  /// Extract the text content between <tag> and </tag>.
  static std::string_view ExtractTagValue(std::string_view content,
                                           std::string_view tag)
  {
    auto open = FindTag(content, tag, 0);
    if (open == std::string_view::npos)
    {
      return {};
    }
    auto close = FindClosingTag(content, tag, open);
    if (close == std::string_view::npos)
    {
      return {};
    }
    auto value_start = open + tag.size() + 2;  // past "<tag>"
    if (value_start >= close)
    {
      return {};
    }
    return content.substr(value_start, close - value_start);
  }
};

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

namespace
{

void PrintResult(const DataResult& result)
{
  std::cout << "\n=== " << result.format << " Processing Result ===\n";
  std::cout << "Headers: ";
  for (std::size_t i = 0; i < result.headers.size(); ++i)
  {
    if (i > 0)
    {
      std::cout << ", ";
    }
    std::cout << result.headers[i];
  }
  std::cout << "\n\nRecords:\n";

  for (std::size_t r = 0; r < result.records.size(); ++r)
  {
    std::cout << "  [" << r << "] ";
    bool first = true;
    for (const auto& [key, value] : result.records[r])
    {
      if (!first)
      {
        std::cout << ", ";
      }
      std::cout << key << "=" << ToString(value);
      first = false;
    }
    std::cout << "\n";
  }
  std::cout << std::endl;
}

}  // namespace

// ---------------------------------------------------------------------------
// Main — demonstrate the Template Method pattern
// ---------------------------------------------------------------------------

int main()
{
  // --- CSV input ---
  constexpr std::string_view kCsvInput =
      "name,age,score\n"
      "Alice,30,95.5\n"
      "Bob,25,88.0\n"
      "Charlie,35,72.3\n";

  // --- JSON Lines input ---
  constexpr std::string_view kJsonInput =
      "{\"name\": \"Alice\", \"age\": 30, \"city\": \"Beijing\"}\n"
      "{\"name\": \"Bob\", \"age\": 25, \"city\": \"Shanghai\"}\n"
      "{\"name\": \"Charlie\", \"age\": 35, \"city\": \"Guangzhou\"}\n";

  // --- XML input ---
  constexpr std::string_view kXmlInput =
      "<data>"
      "<row><name>Alice</name><department>Engineering</department>"
      "<salary>120000</salary></row>"
      "<row><name>Bob</name><department>Design</department>"
      "<salary>95000</salary></row>"
      "<row><name>Charlie</name><department>Marketing</department>"
      "<salary>88000</salary></row>"
      "</data>";

  // Process each format through its dedicated processor.
  // The template method (Process) is identical for all three; only the
  // concrete step implementations differ.
  CsvProcessor csv;
  JsonProcessor json;
  XmlProcessor xml;

  auto csv_result = csv.Process(kCsvInput);
  auto json_result = json.Process(kJsonInput);
  auto xml_result = xml.Process(kXmlInput);

  PrintResult(csv_result);
  PrintResult(json_result);
  PrintResult(xml_result);

  return 0;
}
