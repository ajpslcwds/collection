/**
 * Interpreter Pattern — C++17 Implementation
 *
 * Intent: Define a grammar for a simple arithmetic language and build an
 *         interpreter that evaluates sentences (expressions) in that language.
 *
 * C++17 Features Demonstrated:
 *   - std::optional      for safe variable lookup without sentinel values
 *   - std::variant        for type-safe token representation
 *   - std::string_view    for zero-copy tokenization
 *   - Structured bindings  for clean map/pair unpacking
 *   - if constexpr         for compile-time type dispatch
 *   - Inline variables     for global constant definitions (ODR-safe)
 *   - Fold expressions     for variadic argument validation
 *   - std::shared_ptr      for automatic AST memory management
 *
 * Grammar (EBNF):
 *   expr   -> term (('+' | '-') term)*
 *   term   -> factor (('*' | '/') factor)*
 *   factor -> NUMBER | VARIABLE | '(' expr ')'
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

// ============================================================================
// Forward Declarations
// ============================================================================

class Context;

// ============================================================================
// Abstract Expression — the core abstraction of the Interpreter pattern
// ============================================================================

class Expression
{
 public:
  virtual ~Expression() = default;
  virtual double Interpret(Context& context) const = 0;
  virtual std::string ToString() const = 0;
};

using ExpressionPtr = std::shared_ptr<Expression>;

// ============================================================================
// Context — stores variable bindings
//   Uses C++17 std::optional so callers can distinguish "not set" from 0.0
// ============================================================================

class Context
{
 public:
  void SetVariable(std::string name, double value)
  {
    variables_[std::move(name)] = value;
  }

  // C++17: return std::optional — no magic sentinel, no exception for missing
  std::optional<double> GetVariable(std::string_view name) const
  {
    auto it = variables_.find(std::string(name));
    if (it != variables_.end())
    {
      return it->second;
    }
    return std::nullopt;
  }

  void PrintAll() const
  {
    // C++17: structured bindings for readable map iteration
    for (const auto& [name, value] : variables_)
    {
      std::cout << "  " << name << " = " << value << "\n";
    }
  }

 private:
  std::unordered_map<std::string, double> variables_;
};

// ============================================================================
// Terminal Expressions — leaves of the AST
// ============================================================================

// Literal number: 42, 3.14, etc.
class NumberExpression : public Expression
{
 public:
  explicit NumberExpression(double value) : value_(value) {}

  double Interpret(Context& /*context*/) const override
  {
    return value_;
  }

  std::string ToString() const override
  {
    std::ostringstream oss;
    oss << std::setprecision(10) << value_;
    return oss.str();
  }

 private:
  double value_;
};

// Variable reference: x, y, result, etc.
class VariableExpression : public Expression
{
 public:
  explicit VariableExpression(std::string name) : name_(std::move(name)) {}

  double Interpret(Context& context) const override
  {
    // C++17: structured binding with optional
    if (auto value = context.GetVariable(name_); value.has_value())
    {
      return value.value();
    }
    throw std::runtime_error("Undefined variable: " + name_);
  }

  std::string ToString() const override { return name_; }

 private:
  std::string name_;
};

// ============================================================================
// Nonterminal (Composite) Expressions — interior nodes of the AST
// ============================================================================

// C++17: inline variable — global constant defined in a single place, ODR-safe
inline const std::unordered_map<char, int> kOperatorPrecedence = {
    {'+', 1}, {'-', 1}, {'*', 2}, {'/', 2}};

inline int GetPrecedence(char op)
{
  auto it = kOperatorPrecedence.find(op);
  return (it != kOperatorPrecedence.end()) ? it->second : 0;
}

// Base class for binary operations
class BinaryExpression : public Expression
{
 public:
  BinaryExpression(ExpressionPtr left, ExpressionPtr right, char op)
      : left_(std::move(left)), right_(std::move(right)), op_(op) {}

  std::string ToString() const override
  {
    return "(" + left_->ToString() + " " + op_ + " " + right_->ToString() + ")";
  }

 protected:
  ExpressionPtr left_;
  ExpressionPtr right_;
  char op_;
};

class AddExpression : public BinaryExpression
{
 public:
  using BinaryExpression::BinaryExpression;

  double Interpret(Context& context) const override
  {
    return left_->Interpret(context) + right_->Interpret(context);
  }
};

class SubtractExpression : public BinaryExpression
{
 public:
  using BinaryExpression::BinaryExpression;

  double Interpret(Context& context) const override
  {
    return left_->Interpret(context) - right_->Interpret(context);
  }
};

class MultiplyExpression : public BinaryExpression
{
 public:
  using BinaryExpression::BinaryExpression;

  double Interpret(Context& context) const override
  {
    return left_->Interpret(context) * right_->Interpret(context);
  }
};

class DivideExpression : public BinaryExpression
{
 public:
  using BinaryExpression::BinaryExpression;

  double Interpret(Context& context) const override
  {
    double divisor = right_->Interpret(context);
    if (std::abs(divisor) < 1e-10)
    {
      throw std::runtime_error("Division by zero");
    }
    return left_->Interpret(context) / divisor;
  }
};

// Factory: create the correct BinaryExpression subclass for an operator
ExpressionPtr MakeBinary(ExpressionPtr left, ExpressionPtr right, char op)
{
  switch (op)
  {
    case '+':
      return std::make_shared<AddExpression>(std::move(left), std::move(right), op);
    case '-':
      return std::make_shared<SubtractExpression>(std::move(left), std::move(right), op);
    case '*':
      return std::make_shared<MultiplyExpression>(std::move(left), std::move(right), op);
    case '/':
      return std::make_shared<DivideExpression>(std::move(left), std::move(right), op);
    default:
      throw std::runtime_error(std::string("Unknown operator: ") + op);
  }
}

// ============================================================================
// SumExpression — variadic composite using C++17 fold expressions
// ============================================================================

class SumExpression : public Expression
{
 public:
  template <typename... Args>
  explicit SumExpression(Args&&... args)
      : children_{std::forward<Args>(args)...} {}

  double Interpret(Context& context) const override
  {
    double sum = 0.0;
    for (const auto& child : children_)
    {
      sum += child->Interpret(context);
    }
    return sum;
  }

  std::string ToString() const override
  {
    std::string result = "sum(";
    for (size_t i = 0; i < children_.size(); ++i)
    {
      if (i > 0) result += ", ";
      result += children_[i]->ToString();
    }
    result += ")";
    return result;
  }

 private:
  std::vector<ExpressionPtr> children_;
};

// C++17: fold expression in static_assert validates all args at compile time
template <typename... Args>
ExpressionPtr MakeSum(Args&&... args)
{
  static_assert((std::is_convertible_v<Args, ExpressionPtr> && ...),
                "All arguments must be ExpressionPtr");
  return std::make_shared<SumExpression>(std::forward<Args>(args)...);
}

// ============================================================================
// Tokenizer — converts source text into a token stream
//   Uses C++17 std::string_view for zero-copy references into the source
// ============================================================================

// C++17: std::variant for type-safe token value (number or nothing)
using TokenValue = std::variant<std::monostate, double>;

struct Token
{
  enum class Kind
  {
    kNumber,
    kVariable,
    kOperator,
    kLeftParen,
    kRightParen,
    kEof
  };

  Kind kind;
  std::string_view text;   // C++17: zero-copy view into source string
  TokenValue value;         // C++17: type-safe variant
};

class Tokenizer
{
 public:
  explicit Tokenizer(std::string_view source) : source_(source), pos_(0) {}

  std::vector<Token> Tokenize()
  {
    std::vector<Token> tokens;

    while (pos_ < source_.size())
    {
      SkipWhitespace();
      if (pos_ >= source_.size()) break;

      char ch = source_[pos_];

      if (std::isdigit(ch) || ch == '.')
      {
        tokens.push_back(ReadNumber());
      }
      else if (std::isalpha(ch) || ch == '_')
      {
        tokens.push_back(ReadIdentifier());
      }
      else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
      {
        tokens.push_back({Token::Kind::kOperator, source_.substr(pos_, 1), {}});
        ++pos_;
      }
      else if (ch == '(')
      {
        tokens.push_back({Token::Kind::kLeftParen, source_.substr(pos_, 1), {}});
        ++pos_;
      }
      else if (ch == ')')
      {
        tokens.push_back({Token::Kind::kRightParen, source_.substr(pos_, 1), {}});
        ++pos_;
      }
      else
      {
        throw std::runtime_error(std::string("Unexpected character: ") + ch);
      }
    }

    tokens.push_back({Token::Kind::kEof, {}, {}});
    return tokens;
  }

 private:
  void SkipWhitespace()
  {
    while (pos_ < source_.size() && std::isspace(source_[pos_]))
    {
      ++pos_;
    }
  }

  Token ReadNumber()
  {
    size_t start = pos_;
    while (pos_ < source_.size() && (std::isdigit(source_[pos_]) || source_[pos_] == '.'))
    {
      ++pos_;
    }
    std::string_view text = source_.substr(start, pos_ - start);
    std::string temp(text);  // stod needs null-terminated string
    double numeric = std::stod(temp);
    return {Token::Kind::kNumber, text, numeric};
  }

  Token ReadIdentifier()
  {
    size_t start = pos_;
    while (pos_ < source_.size() && (std::isalnum(source_[pos_]) || source_[pos_] == '_'))
    {
      ++pos_;
    }
    return {Token::Kind::kVariable, source_.substr(start, pos_ - start), {}};
  }

  std::string_view source_;
  size_t pos_;
};

// ============================================================================
// Recursive Descent Parser — builds the AST from tokens
// ============================================================================
//
// Grammar (operator precedence encoded in the rule structure):
//   expr   -> term (('+' | '-') term)*
//   term   -> factor (('*' | '/') factor)*
//   factor -> NUMBER | VARIABLE | '(' expr ')'

class Parser
{
 public:
  explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}

  ExpressionPtr Parse()
  {
    auto result = ParseExpr();
    if (Current().kind != Token::Kind::kEof)
    {
      throw std::runtime_error("Unexpected token at end of expression");
    }
    return result;
  }

 private:
  // expr -> term (('+' | '-') term)*
  ExpressionPtr ParseExpr()
  {
    auto left = ParseTerm();

    while (Current().kind == Token::Kind::kOperator &&
           (Current().text[0] == '+' || Current().text[0] == '-'))
    {
      char op = Current().text[0];
      Advance();
      auto right = ParseTerm();
      left = MakeBinary(std::move(left), std::move(right), op);
    }

    return left;
  }

  // term -> factor (('*' | '/') factor)*
  ExpressionPtr ParseTerm()
  {
    auto left = ParseFactor();

    while (Current().kind == Token::Kind::kOperator &&
           (Current().text[0] == '*' || Current().text[0] == '/'))
    {
      char op = Current().text[0];
      Advance();
      auto right = ParseFactor();
      left = MakeBinary(std::move(left), std::move(right), op);
    }

    return left;
  }

  // factor -> NUMBER | VARIABLE | '(' expr ')'
  ExpressionPtr ParseFactor()
  {
    const Token& tok = Current();

    if (tok.kind == Token::Kind::kNumber)
    {
      // C++17: std::get to extract value from variant
      double numeric = std::get<double>(tok.value);
      Advance();
      return std::make_shared<NumberExpression>(numeric);
    }

    if (tok.kind == Token::Kind::kVariable)
    {
      std::string name(tok.text);
      Advance();
      return std::make_shared<VariableExpression>(std::move(name));
    }

    if (tok.kind == Token::Kind::kLeftParen)
    {
      Advance();  // skip '('
      auto expr = ParseExpr();
      if (Current().kind != Token::Kind::kRightParen)
      {
        throw std::runtime_error("Expected ')'");
      }
      Advance();  // skip ')'
      return expr;
    }

    throw std::runtime_error("Unexpected token in factor");
  }

  const Token& Current() const { return tokens_[pos_]; }

  void Advance()
  {
    if (pos_ < tokens_.size()) ++pos_;
  }

  const std::vector<Token>& tokens_;
  size_t pos_;
};

// ============================================================================
// Helper: parse + evaluate in one call
//   C++17: if constexpr selects return type at compile time
// ============================================================================

template <typename ResultType = double>
ResultType Evaluate(std::string_view expression, Context& context)
{
  Tokenizer tokenizer(expression);
  auto tokens = tokenizer.Tokenize();

  Parser parser(tokens);
  auto ast = parser.Parse();

  // C++17: if constexpr — compile-time branch, dead code is discarded
  if constexpr (std::is_same_v<ResultType, double>)
  {
    return ast->Interpret(context);
  }
  else if constexpr (std::is_same_v<ResultType, std::string>)
  {
    return ast->ToString();
  }
}

// ============================================================================
// main — demonstrate the Interpreter pattern
// ============================================================================

int main()
{
  std::cout << "=== Interpreter Pattern (C++17) ===\n\n";

  // --- Set up context with some variables ---
  Context context;
  context.SetVariable("x", 10.0);
  context.SetVariable("y", 3.0);
  context.SetVariable("z", 7.0);

  std::cout << "[Variables]\n";
  context.PrintAll();
  std::cout << "\n";

  // --- 1. Basic arithmetic: precedence handled by grammar rules ---
  std::cout << "--- 1. Basic Arithmetic (operator precedence) ---\n";
  {
    std::string_view expr = "3 + 4 * 2 - 1";
    std::cout << "  Expression : " << expr << "\n";

    double result = Evaluate(expr, context);
    std::cout << "  Result     : " << result << "\n";

    // C++17: if constexpr — same function, different return type
    std::string ast = Evaluate<std::string>(expr, context);
    std::cout << "  AST        : " << ast << "\n";
    std::cout << "  Expected   : ((3 + (4 * 2)) - 1) = 10\n\n";
  }

  // --- 2. Expressions with variables ---
  std::cout << "--- 2. Variable References ---\n";
  {
    std::string_view expr = "x * y + z";
    std::cout << "  Expression : " << expr << "\n";
    std::cout << "  Bindings   : x=10, y=3, z=7\n";

    double result = Evaluate(expr, context);
    std::cout << "  Result     : " << result << "\n";

    std::string ast = Evaluate<std::string>(expr, context);
    std::cout << "  AST        : " << ast << "\n";
    std::cout << "  Expected   : ((10 * 3) + 7) = 37\n\n";
  }

  // --- 3. Parenthesized expressions ---
  std::cout << "--- 3. Parenthesized Expressions ---\n";
  {
    std::string_view expr = "(x + y) * (z - 1)";
    std::cout << "  Expression : " << expr << "\n";

    double result = Evaluate(expr, context);
    std::cout << "  Result     : " << result << "\n";

    std::string ast = Evaluate<std::string>(expr, context);
    std::cout << "  AST        : " << ast << "\n";
    std::cout << "  Expected   : ((10 + 3) * (7 - 1)) = 78\n\n";
  }

  // --- 4. Manual AST construction (no parser) ---
  std::cout << "--- 4. Manual AST Construction ---\n";
  {
    // Build the tree for (x + 5) * 2 by hand
    auto var_x = std::make_shared<VariableExpression>("x");
    auto num_5 = std::make_shared<NumberExpression>(5.0);
    auto num_2 = std::make_shared<NumberExpression>(2.0);

    auto add = MakeBinary(var_x, num_5, '+');
    auto mul = MakeBinary(add, num_2, '*');

    std::cout << "  AST        : " << mul->ToString() << "\n";
    std::cout << "  Result     : " << mul->Interpret(context) << "\n";
    std::cout << "  Expected   : ((10 + 5) * 2) = 30\n\n";
  }

  // --- 5. Variadic SumExpression with C++17 fold expressions ---
  std::cout << "--- 5. Variadic Sum (C++17 fold expression) ---\n";
  {
    auto a = std::make_shared<NumberExpression>(1.0);
    auto b = std::make_shared<NumberExpression>(2.0);
    auto c = std::make_shared<NumberExpression>(3.0);
    auto d = std::make_shared<NumberExpression>(4.0);
    auto vx = std::make_shared<VariableExpression>("x");

    // MakeSum uses a fold expression in its static_assert
    auto sum_expr = MakeSum(a, b, c, d, vx);

    std::cout << "  Expression : " << sum_expr->ToString() << "\n";
    std::cout << "  Result     : " << sum_expr->Interpret(context) << "\n";
    std::cout << "  Expected   : sum(1, 2, 3, 4, x) = 20\n\n";
  }

  // --- 6. std::optional for safe variable lookup ---
  std::cout << "--- 6. std::optional Variable Lookup ---\n";
  {
    // C++17: structured binding with a local pair
    auto result_x = context.GetVariable("x");
    auto result_w = context.GetVariable("w");  // does not exist

    auto PrintVar = [](std::string_view name, std::optional<double> val)
    {
      std::cout << "  Variable '" << name << "': ";
      if (val.has_value())
      {
        std::cout << val.value() << "\n";
      }
      else
      {
        std::cout << "std::nullopt (not defined)\n";
      }
    };

    PrintVar("x", result_x);
    PrintVar("w", result_w);
    std::cout << "\n";
  }

  // --- 7. Inline variable: operator precedence table ---
  std::cout << "--- 7. Inline Operator Precedence Table ---\n";
  {
    // C++17: structured bindings to iterate the inline map
    for (const auto& [op, prec] : kOperatorPrecedence)
    {
      std::cout << "  '" << op << "' -> precedence " << prec << "\n";
    }
    std::cout << "\n";
  }

  // --- 8. Error handling ---
  std::cout << "--- 8. Error Handling ---\n";
  {
    // Undefined variable in expression
    try
    {
      std::string_view expr = "missing + 1";
      Evaluate(expr, context);
    }
    catch (const std::runtime_error& e)
    {
      std::cout << "  Undefined variable caught: " << e.what() << "\n";
    }

    // Division by zero
    try
    {
      context.SetVariable("zero", 0.0);
      std::string_view expr = "10 / zero";
      Evaluate(expr, context);
    }
    catch (const std::runtime_error& e)
    {
      std::cout << "  Division by zero caught  : " << e.what() << "\n";
    }
  }

  std::cout << "\n=== Done ===\n";
  return 0;
}
