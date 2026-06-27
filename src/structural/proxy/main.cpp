/**
 * Proxy（代理模式）
 *
 * 意图：为其他对象提供一种代理以控制对这个对象的访问。
 *
 * C++17 特性：
 *   - std::shared_mutex / std::shared_lock / std::unique_lock（读写锁保护缓存）
 *   - std::optional（延迟加载的缓存值）
 *   - std::variant + std::visit（统一代理类型集合）
 *   - if constexpr（编译期策略分支）
 *   - 结构化绑定（structured bindings）
 *   - inline 变量（类内静态常量）
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <variant>
#include <vector>

// =============================================================================
//  Subject 抽象接口
// =============================================================================

class Image
{
 public:
  virtual ~Image() = default;
  virtual void Display() const = 0;
  virtual std::string GetFilename() const = 0;
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;
};

// =============================================================================
//  RealSubject —— 真实对象，开销较大
// =============================================================================

class HighResolutionImage : public Image
{
 public:
  explicit HighResolutionImage(std::string filename)
      : filename_(std::move(filename))
  {
    // 模拟从磁盘/网络加载高分辨率图片的昂贵操作
    std::cout << "  [HighResolutionImage] 正在从磁盘加载 \"" << filename_
              << "\" ...\n";
    width_ = 3840;
    height_ = 2160;
    loaded_ = true;
    std::cout << "  [HighResolutionImage] 加载完成 (" << width_ << "x"
              << height_ << ")\n";
  }

  void Display() const override
  {
    std::cout << "  [HighResolutionImage] 显示 \"" << filename_ << "\" ("
              << width_ << "x" << height_ << ")\n";
  }

  std::string GetFilename() const override { return filename_; }
  int GetWidth() const override { return width_; }
  int GetHeight() const override { return height_; }

 private:
  std::string filename_;
  int width_ = 0;
  int height_ = 0;
  bool loaded_ = false;
};

// =============================================================================
//  代理 1：虚拟代理（Virtual Proxy）—— 延迟初始化
// =============================================================================

class VirtualProxyImage : public Image
{
 public:
  explicit VirtualProxyImage(std::string filename)
      : filename_(std::move(filename))
  {
  }

  void Display() const override
  {
    // 首次调用时才加载真实图片
    if (!real_image_.has_value())
    {
      std::cout << "  [VirtualProxy] 首次访问，触发延迟加载\n";
      real_image_.emplace(filename_);
    }
    real_image_->Display();
  }

  std::string GetFilename() const override { return filename_; }

  int GetWidth() const override
  {
    EnsureLoaded();
    return real_image_->GetWidth();
  }

  int GetHeight() const override
  {
    EnsureLoaded();
    return real_image_->GetHeight();
  }

 private:
  void EnsureLoaded() const
  {
    if (!real_image_.has_value())
    {
      real_image_.emplace(filename_);
    }
  }

  std::string filename_;
  mutable std::optional<HighResolutionImage> real_image_;
};

// =============================================================================
//  代理 2：保护代理（Protection Proxy）—— 权限控制
// =============================================================================

enum class AccessLevel
{
  kGuest,
  kUser,
  kAdmin
};

inline std::string ToString(AccessLevel level)
{
  switch (level)
  {
    case AccessLevel::kGuest:
      return "Guest";
    case AccessLevel::kUser:
      return "User";
    case AccessLevel::kAdmin:
      return "Admin";
  }
  return "Unknown";
}

class ProtectionProxyImage : public Image
{
 public:
  ProtectionProxyImage(std::shared_ptr<Image> real_image,
                       AccessLevel required_level,
                       AccessLevel current_level)
      : real_image_(std::move(real_image)),
        required_level_(required_level),
        current_level_(current_level)
  {
  }

  void Display() const override
  {
    if (current_level_ < required_level_)
    {
      std::cout << "  [ProtectionProxy] 拒绝访问！需要权限 "
                << ToString(required_level_) << "，当前权限 "
                << ToString(current_level_) << "\n";
      return;
    }
    std::cout << "  [ProtectionProxy] 权限校验通过，转发请求\n";
    real_image_->Display();
  }

  std::string GetFilename() const override
  {
    return real_image_->GetFilename();
  }

  int GetWidth() const override { return real_image_->GetWidth(); }
  int GetHeight() const override { return real_image_->GetHeight(); }

 private:
  std::shared_ptr<Image> real_image_;
  AccessLevel required_level_;
  AccessLevel current_level_;
};

// =============================================================================
//  代理 3：缓存代理（Caching Proxy）—— 线程安全的缓存
// =============================================================================

class CachingProxyImage : public Image
{
 public:
  explicit CachingProxyImage(std::shared_ptr<Image> real_image)
      : real_image_(std::move(real_image))
  {
  }

  void Display() const override
  {
    {
      std::shared_lock<std::shared_mutex> lock(*cache_mutex_);
      if (display_cache_.has_value())
      {
        std::cout << "  [CachingProxy] 命中缓存，直接返回\n";
        std::cout << *display_cache_;
        ++cache_hits_;
        return;
      }
    }

    // 缓存未命中，获取独占锁后写入
    {
      std::unique_lock<std::shared_mutex> lock(*cache_mutex_);
      // double-check：另一个线程可能已写入
      if (display_cache_.has_value())
      {
        std::cout << "  [CachingProxy] 命中缓存（double-check），直接返回\n";
        std::cout << *display_cache_;
        ++cache_hits_;
        return;
      }
      std::cout << "  [CachingProxy] 缓存未命中，调用真实对象\n";
      std::string output;
      output += "  [CachingProxy:cached] ";
      output += real_image_->GetFilename();
      output += " 已显示\n";
      real_image_->Display();
      display_cache_ = output;
      ++cache_misses_;
    }
  }

  std::string GetFilename() const override
  {
    return real_image_->GetFilename();
  }

  int GetWidth() const override { return real_image_->GetWidth(); }
  int GetHeight() const override { return real_image_->GetHeight(); }

  // 获取缓存统计信息（结构化绑定友好的接口）
  std::pair<int, int> GetCacheStats() const
  {
    std::shared_lock<std::shared_mutex> lock(*cache_mutex_);
    return {cache_hits_, cache_misses_};
  }

  void InvalidateCache()
  {
    std::unique_lock<std::shared_mutex> lock(*cache_mutex_);
    display_cache_.reset();
    std::cout << "  [CachingProxy] 缓存已清除\n";
  }

 private:
  std::shared_ptr<Image> real_image_;
  mutable std::unique_ptr<std::shared_mutex> cache_mutex_ =
      std::make_unique<std::shared_mutex>();
  mutable std::optional<std::string> display_cache_;
  mutable int cache_hits_ = 0;
  mutable int cache_misses_ = 0;
};

// =============================================================================
//  使用 std::variant 统一代理类型集合
// =============================================================================

using ImageProxy = std::variant<VirtualProxyImage,
                                ProtectionProxyImage,
                                CachingProxyImage>;

// 编译期多态：if constexpr 分发
template <typename ProxyType>
void InvokeDisplay(const ProxyType& proxy)
{
  if constexpr (std::is_same_v<ProxyType, CachingProxyImage>)
  {
    proxy.Display();
    auto [hits, misses] = proxy.GetCacheStats();
    std::cout << "  [Stats] 缓存命中: " << hits << ", 未命中: " << misses
              << "\n";
  }
  else
  {
    proxy.Display();
  }
}

// 通过 std::visit 访问 variant 中的任意代理
void DisplayProxy(const ImageProxy& proxy)
{
  std::visit(
      [](const auto& p)
      {
        InvokeDisplay(p);
      },
      proxy);
}

// =============================================================================
//  main —— 演示各种代理
// =============================================================================

int main()
{
  std::cout << "========================================\n";
  std::cout << "  Proxy（代理模式）演示\n";
  std::cout << "========================================\n\n";

  // ---------- 1. 虚拟代理：延迟加载 ----------
  std::cout << "--- 1. 虚拟代理（Virtual Proxy）：延迟加载 ---\n";
  VirtualProxyImage virtual_proxy("vacation_4k.jpg");
  std::cout << "图片对象已创建，但尚未加载实际文件\n";
  std::cout << "首次调用 Display()：\n";
  virtual_proxy.Display();
  std::cout << "再次调用 Display()（对象已存在）：\n";
  virtual_proxy.Display();
  std::cout << "\n";

  // ---------- 2. 保护代理：权限控制 ----------
  std::cout << "--- 2. 保护代理（Protection Proxy）：权限控制 ---\n";
  auto real_image =
      std::make_shared<HighResolutionImage>("secret_document.png");

  ProtectionProxyImage admin_proxy(real_image, AccessLevel::kAdmin,
                                   AccessLevel::kAdmin);
  ProtectionProxyImage guest_proxy(real_image, AccessLevel::kAdmin,
                                   AccessLevel::kGuest);

  std::cout << "Admin 访问：\n";
  admin_proxy.Display();
  std::cout << "Guest 访问：\n";
  guest_proxy.Display();
  std::cout << "\n";

  // ---------- 3. 缓存代理：结果缓存 ----------
  std::cout << "--- 3. 缓存代理（Caching Proxy）：线程安全缓存 ---\n";
  auto image_for_cache =
      std::make_shared<HighResolutionImage>("landscape.png");
  CachingProxyImage caching_proxy(image_for_cache);

  std::cout << "第一次调用（缓存未命中）：\n";
  caching_proxy.Display();
  std::cout << "第二次调用（缓存命中）：\n";
  caching_proxy.Display();
  std::cout << "第三次调用（缓存命中）：\n";
  caching_proxy.Display();

  {
    auto [hits, misses] = caching_proxy.GetCacheStats();
    std::cout << "缓存统计 -> 命中: " << hits << ", 未命中: " << misses << "\n";
  }

  caching_proxy.InvalidateCache();
  std::cout << "清除缓存后再次调用：\n";
  caching_proxy.Display();
  std::cout << "\n";

  // ---------- 4. std::variant 统一代理集合 ----------
  std::cout << "--- 4. std::variant 统一代理集合 ---\n";
  std::vector<ImageProxy> proxies;
  proxies.emplace_back(VirtualProxyImage("photo1.jpg"));
  proxies.emplace_back(ProtectionProxyImage(
      real_image, AccessLevel::kUser, AccessLevel::kUser));
  proxies.emplace_back(CachingProxyImage(
      std::make_shared<HighResolutionImage>("photo2.jpg")));

  for (const auto& proxy : proxies)
  {
    DisplayProxy(proxy);
    std::cout << "---\n";
  }

  std::cout << "\n演示结束。\n";
  return 0;
}
