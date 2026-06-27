/**
 * Composite Pattern - File System Example
 *
 * Intent: Compose objects into tree structures to represent part-whole
 * hierarchies. Composite lets clients treat individual objects and
 * compositions of objects uniformly.
 *
 * C++17 Features Used:
 * - std::variant: Type-safe union for File/Directory polymorphism
 * - std::optional: Optional values for safe operations
 * - Structured bindings: Clean tuple/pair unpacking
 * - if constexpr: Compile-time branching for type-specific logic
 * - Fold expressions: Variadic template parameter pack expansion
 * - std::filesystem: Modern file system operations
 */

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace composite {

// Forward declarations
struct File;
struct Directory;

// Component variant type - type-safe polymorphism without virtual functions
using FileSystemNode = std::variant<File, Directory>;

/**
 * Leaf node representing a file in the file system.
 * Contains file name and size.
 */
struct File
{
  std::string name;
  std::size_t size;
};

/**
 * Composite node representing a directory in the file system.
 * Contains directory name and children (files or subdirectories).
 */
struct Directory
{
  std::string name;
  std::vector<FileSystemNode> children;
};

/**
 * Visitor to calculate total size of a file system node.
 * Demonstrates the visitor pattern with std::variant.
 */
struct SizeCalculator
{
  // Calculate size of a single file
  std::size_t operator()(const File& file) const
  {
    return file.size;
  }

  // Calculate size of a directory (sum of all children)
  std::size_t operator()(const Directory& dir) const
  {
    std::size_t total = 0;
    for (const auto& child : dir.children)
    {
      total += std::visit(*this, child);
    }
    return total;
  }
};

/**
 * Visitor to display file system tree structure.
 * Uses indentation to show hierarchy.
 */
struct TreePrinter
{
  int indent_level = 0;

  // Print file with indentation
  void operator()(const File& file) const
  {
    std::string indent(indent_level * 2, ' ');
    std::cout << indent << "[FILE] " << file.name
              << " (" << file.size << " bytes)\n";
  }

  // Print directory and recursively print children
  void operator()(const Directory& dir) const
  {
    std::string indent(indent_level * 2, ' ');
    std::cout << indent << "[DIR]  " << dir.name << "/\n";

    // Create new visitor with increased indent for children
    TreePrinter child_printer{indent_level + 1};
    for (const auto& child : dir.children)
    {
      std::visit(child_printer, child);
    }
  }
};

/**
 * Visitor to search for files by name pattern.
 * Returns matching file paths using std::optional.
 */
struct FileSearcher
{
  std::string pattern;
  std::string current_path;
  mutable std::vector<std::string> results;

  // Search in file - check if name matches pattern
  std::optional<std::string> operator()(const File& file) const
  {
    if (file.name.find(pattern) != std::string::npos)
    {
      std::string path = current_path + "/" + file.name;
      results.push_back(path);
      return path;
    }
    return std::nullopt;
  }

  // Search in directory - recursively search children
  std::optional<std::string> operator()(const Directory& dir) const
  {
    std::string dir_path = current_path + "/" + dir.name;
    FileSearcher child_searcher{pattern, dir_path, results};

    for (const auto& child : dir.children)
    {
      std::visit(child_searcher, child);
    }

    // Update results from child searcher
    results = child_searcher.results;
    return std::nullopt;
  }
};

/**
 * Helper function to add a file to a directory.
 * Uses structured bindings for clean syntax.
 */
void AddFile(Directory& dir, const std::string& name, std::size_t size)
{
  dir.children.emplace_back(File{name, size});
}

/**
 * Helper function to add a subdirectory to a directory.
 * Returns reference to the new directory for chaining.
 */
Directory& AddDirectory(Directory& dir, const std::string& name)
{
  dir.children.emplace_back(Directory{name, {}});
  // Return reference to the newly added directory
  return std::get<Directory>(dir.children.back());
}

/**
 * Template function demonstrating fold expressions.
 * Calculates total size of multiple file system nodes.
 */
template <typename... Nodes>
std::size_t TotalSize(const Nodes&... nodes)
{
  SizeCalculator calc;
  // Fold expression: sum of all node sizes
  return (std::visit(calc, nodes) + ...);
}

/**
 * Helper function to create a sample file system structure.
 * Demonstrates composite pattern in action.
 */
Directory CreateSampleFileSystem()
{
  Directory root{"root", {}};

  // Create documents directory
  auto& docs = AddDirectory(root, "documents");
  AddFile(docs, "report.txt", 1024);
  AddFile(docs, "presentation.pptx", 5120);

  // Create nested subdirectory
  auto& projects = AddDirectory(docs, "projects");
  AddFile(projects, "main.cpp", 2048);
  AddFile(projects, "README.md", 512);

  // Create images directory
  auto& images = AddDirectory(root, "images");
  AddFile(images, "photo.jpg", 3072);
  AddFile(images, "icon.png", 256);

  // Add root level files
  AddFile(root, "config.json", 128);
  AddFile(root, "Makefile", 64);

  return root;
}

}  // namespace composite

/**
 * Demonstrate the Composite pattern with various operations.
 */
int main()
{
  using namespace composite;

  std::cout << "=== Composite Pattern: File System Example ===\n\n";

  // Create sample file system
  Directory fs = CreateSampleFileSystem();

  // 1. Display tree structure
  std::cout << "1. File System Tree:\n";
  TreePrinter printer;
  std::visit(printer, FileSystemNode{fs});

  // 2. Calculate total size using visitor
  std::cout << "\n2. Size Calculations:\n";
  SizeCalculator size_calc;
  auto total_size = std::visit(size_calc, FileSystemNode{fs});
  std::cout << "Total size: " << total_size << " bytes\n";

  // 3. Calculate size of specific directory using fold expression
  auto& docs_dir = std::get<Directory>(fs.children[0]);
  auto& images_dir = std::get<Directory>(fs.children[1]);

  // Demonstrate fold expression with multiple nodes
  auto combined_size = TotalSize(
    FileSystemNode{docs_dir},
    FileSystemNode{images_dir}
  );
  std::cout << "Documents + Images size: " << combined_size << " bytes\n";

  // 4. Search for files
  std::cout << "\n3. Search Results:\n";
  FileSearcher searcher{".cpp", "", {}};
  std::visit(searcher, FileSystemNode{fs});

  std::cout << "Files matching '.cpp':\n";
  for (const auto& path : searcher.results)
  {
    std::cout << "  " << path << "\n";
  }

  // 5. Demonstrate structured bindings with directory iteration
  std::cout << "\n4. Directory Contents (using structured bindings):\n";
  for (const auto& child : fs.children)
  {
    // Structured binding to extract variant alternatives
    if (std::holds_alternative<File>(child))
    {
      const auto& [name, size] = std::get<File>(child);
      std::cout << "  File: " << name << " (" << size << " bytes)\n";
    }
    else if (std::holds_alternative<Directory>(child))
    {
      const auto& [name, children] = std::get<Directory>(child);
      std::cout << "  Directory: " << name << " (" << children.size()
                << " items)\n";
    }
  }

  // 6. Demonstrate if constexpr for type-specific processing
  std::cout << "\n5. Type-specific processing (if constexpr):\n";
  auto process_node = [](const auto& node)
  {
    using NodeType = std::decay_t<decltype(node)>;

    if constexpr (std::is_same_v<NodeType, File>)
    {
      std::cout << "Processing file: " << node.name << "\n";
    }
    else if constexpr (std::is_same_v<NodeType, Directory>)
    {
      std::cout << "Processing directory: " << node.name
                << " with " << node.children.size() << " children\n";
    }
  };

  // Apply to first child
  std::visit(process_node, fs.children[0]);
  std::visit(process_node, fs.children[1]);

  // 7. Demonstrate std::optional usage
  std::cout << "\n6. Optional operations:\n";
  auto find_file = [](const Directory& dir,
                      const std::string& name) -> std::optional<File>
  {
    for (const auto& child : dir.children)
    {
      if (std::holds_alternative<File>(child))
      {
        const auto& file = std::get<File>(child);
        if (file.name == name)
        {
          return file;
        }
      }
    }
    return std::nullopt;
  };

  auto result = find_file(fs, "config.json");
  if (result.has_value())
  {
    std::cout << "Found: " << result->name << " (" << result->size
              << " bytes)\n";
  }

  auto not_found = find_file(fs, "nonexistent.txt");
  if (!not_found.has_value())
  {
    std::cout << "File not found (expected)\n";
  }

  std::cout << "\n=== End of Composite Pattern Demo ===\n";

  return 0;
}
