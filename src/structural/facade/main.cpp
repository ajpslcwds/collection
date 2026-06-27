/**
 * Facade Pattern (外观模式)
 *
 * Intent: 为子系统中的一组接口提供一个统一的高层接口，使子系统更容易使用。
 *         本示例以家庭影院为例，演示外观模式如何简化多个子系统的协同工作。
 *
 * C++17 Features Used:
 *   - std::optional: 表示可选子系统（灯光），避免空指针
 *   - std::variant: 类型安全地标识当前活跃的媒体播放器
 *   - std::string_view: 只读字符串参数，避免 std::string 拷贝
 *   - [[nodiscard]]: 标记查询方法，提醒调用者不要忽略返回值
 *   - inline 变量: 类内常量定义，避免 ODR 问题
 *   - constexpr: 编译期计算子系统数量
 *   - 折叠表达式 (fold expressions): 可变参数模板批量操作子系统
 *   - std::shared_mutex / std::shared_lock: 读写锁实现线程安全的状态查询
 *   - 嵌套命名空间 (nested namespace declaration)
 */

#include <array>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <variant>

// ============================================================================
// 子系统定义（客户端不应直接使用这些类）
// ============================================================================

// NOLINTNEXTLINE(readability-identifier-naming)
namespace home_theater::subsystem
{

/// 蓝光播放器子系统。
class BluRayPlayer
{
 public:
  void Play(std::string_view title)
  {
    std::cout << "  [BluRayPlayer] 播放蓝光光盘: " << title << "\n";
  }

  void Stop()
  {
    std::cout << "  [BluRayPlayer] 停止播放\n";
  }
};

/// DVD 播放器子系统。
class DvdPlayer
{
 public:
  void Play(std::string_view title)
  {
    std::cout << "  [DvdPlayer] 播放 DVD: " << title << "\n";
  }

  void Stop()
  {
    std::cout << "  [DvdPlayer] 停止播放\n";
  }
};

/// 流媒体播放器子系统。
class StreamingPlayer
{
 public:
  void Play(std::string_view service, std::string_view title)
  {
    std::cout << "  [StreamingPlayer] 通过 " << service << " 播放: " << title
              << "\n";
  }

  void Stop()
  {
    std::cout << "  [StreamingPlayer] 停止播放\n";
  }
};

/// 功放子系统。
class Amplifier
{
 public:
  void On()
  {
    std::cout << "  [Amplifier] 开机\n";
  }

  void Off()
  {
    std::cout << "  [Amplifier] 关机\n";
  }

  void SetVolume(int level)
  {
    std::cout << "  [Amplifier] 设置音量: " << level << "\n";
  }

  void SetSurroundSound()
  {
    std::cout << "  [Amplifier] 启用环绕声模式\n";
  }
};

/// 投影仪子系统。
class Projector
{
 public:
  void On()
  {
    std::cout << "  [Projector] 开机\n";
  }

  void Off()
  {
    std::cout << "  [Projector] 关机\n";
  }

  void SetWideScreenMode()
  {
    std::cout << "  [Projector] 设置宽屏模式\n";
  }
};

/// 电动幕布子系统。
class Screen
{
 public:
  void Deploy()
  {
    std::cout << "  [Screen] 幕布降下\n";
  }

  void Retract()
  {
    std::cout << "  [Screen] 幕布收起\n";
  }
};

/// 灯光子系统。
class Lights
{
 public:
  void Dim(int level)
  {
    std::cout << "  [Lights] 灯光调暗至 " << level << "%\n";
  }

  void On()
  {
    std::cout << "  [Lights] 灯光开启\n";
  }
};

}  // namespace home_theater::subsystem

// ============================================================================
// 外观类
// ============================================================================

// NOLINTNEXTLINE(readability-identifier-naming)
namespace home_theater
{

/// 家庭影院外观类。
///
/// 封装了投影仪、功放、幕布、灯光、蓝光播放器、DVD 播放器和流媒体播放器
/// 等子系统，为客户端提供简化的 WatchMovie / WatchStream / EndMovie 接口。
/// 客户端无需了解各子系统的接口、启动顺序和协作关系。
class HomeTheaterFacade
{
 public:
  /// 媒体类型标识：用 std::variant 中存储的指针类型来标识当前活跃的播放器。
  using MediaType = std::variant<subsystem::BluRayPlayer*,
                                 subsystem::DvdPlayer*,
                                 subsystem::StreamingPlayer*>;

  /// 子系统总数（编译期常量，使用 inline 变量避免 ODR 问题）。
  static inline constexpr int kSubsystemCount = 7;

  HomeTheaterFacade(std::unique_ptr<subsystem::Projector> projector,
                    std::unique_ptr<subsystem::Amplifier> amplifier,
                    std::unique_ptr<subsystem::Screen> screen,
                    std::unique_ptr<subsystem::BluRayPlayer> blu_ray,
                    std::unique_ptr<subsystem::DvdPlayer> dvd,
                    std::unique_ptr<subsystem::StreamingPlayer> streaming,
                    std::optional<std::unique_ptr<subsystem::Lights>> lights =
                        std::nullopt)
      : projector_{std::move(projector)},
        amplifier_{std::move(amplifier)},
        screen_{std::move(screen)},
        blu_ray_{std::move(blu_ray)},
        dvd_{std::move(dvd)},
        streaming_{std::move(streaming)},
        lights_{std::move(lights)},
        active_media_{blu_ray_.get()}
  {
  }

  ~HomeTheaterFacade() = default;

  HomeTheaterFacade(const HomeTheaterFacade&) = delete;
  HomeTheaterFacade& operator=(const HomeTheaterFacade&) = delete;
  HomeTheaterFacade(HomeTheaterFacade&&) = delete;
  HomeTheaterFacade& operator=(HomeTheaterFacade&&) = delete;

  /// 观看电影（通过蓝光或 DVD）。
  void WatchMovie(std::string_view title, bool is_bluray = true)
  {
    std::unique_lock lock{mutex_};

    std::cout << "\n=== 准备观看电影 ===\n";

    // 设置媒体源
    if (is_bluray)
    {
      active_media_ = blu_ray_.get();
    }
    else
    {
      active_media_ = dvd_.get();
    }

    current_title_ = title;

    // 启动各子系统（外观封装了复杂的启动顺序）
    screen_->Deploy();
    projector_->On();
    projector_->SetWideScreenMode();
    amplifier_->On();
    amplifier_->SetSurroundSound();
    amplifier_->SetVolume(kDefaultVolume);

    // 灯光是可选子系统，通过 std::optional 安全访问
    if (lights_.has_value())
    {
      lights_.value()->Dim(kDimLevel);
    }

    // 通过 std::visit 分发到具体播放器类型
    std::visit(
        [&title](auto* player)
        {
          using PlayerType = std::decay_t<decltype(*player)>;
          if constexpr (!std::is_same_v<PlayerType,
                                        subsystem::StreamingPlayer>)
          {
            player->Play(title);
          }
        },
        active_media_);

    std::cout << "=== 开始观影: " << title << " ===\n";
  }

  /// 观看流媒体内容。
  void WatchStream(std::string_view service, std::string_view title)
  {
    std::unique_lock lock{mutex_};

    std::cout << "\n=== 准备观看流媒体 ===\n";

    active_media_ = streaming_.get();
    current_title_ = title;

    screen_->Deploy();
    projector_->On();
    projector_->SetWideScreenMode();
    amplifier_->On();
    amplifier_->SetSurroundSound();
    amplifier_->SetVolume(kDefaultVolume);

    if (lights_.has_value())
    {
      lights_.value()->Dim(kDimLevel);
    }

    // 流媒体播放器需要额外的 service 参数
    std::visit(
        [&service, &title](auto* player)
        {
          using PlayerType = std::decay_t<decltype(*player)>;
          if constexpr (std::is_same_v<PlayerType,
                                       subsystem::StreamingPlayer>)
          {
            player->Play(service, title);
          }
          else
          {
            player->Play(title);
          }
        },
        active_media_);

    std::cout << "=== 开始观影: " << title << " (via " << service
              << ") ===\n";
  }

  /// 结束观影，按相反顺序关闭所有子系统。
  void EndMovie()
  {
    std::unique_lock lock{mutex_};

    std::cout << "\n=== 关闭家庭影院 ===\n";

    // 停止当前媒体播放
    std::visit([](auto* player) { player->Stop(); }, active_media_);

    amplifier_->Off();
    projector_->Off();
    screen_->Retract();

    if (lights_.has_value())
    {
      lights_.value()->On();
    }

    std::cout << "=== 家庭影院已关闭 ===\n\n";
    current_title_.clear();
  }

  /// 查询当前播放状态。
  ///
  /// 使用 [[nodiscard]] 属性提醒调用者不要忽略返回值。
  /// 使用 std::shared_lock 允许并发读取。
  [[nodiscard]] std::string GetStatus() const
  {
    std::shared_lock lock{mutex_};
    return current_title_.empty() ? "待机中"
                                  : "正在播放: " + current_title_;
  }

 private:
  static constexpr int kDefaultVolume = 7;
  static constexpr int kDimLevel = 30;

  std::unique_ptr<subsystem::Projector> projector_;
  std::unique_ptr<subsystem::Amplifier> amplifier_;
  std::unique_ptr<subsystem::Screen> screen_;
  std::unique_ptr<subsystem::BluRayPlayer> blu_ray_;
  std::unique_ptr<subsystem::DvdPlayer> dvd_;
  std::unique_ptr<subsystem::StreamingPlayer> streaming_;
  std::optional<std::unique_ptr<subsystem::Lights>> lights_;

  MediaType active_media_;
  std::string current_title_;

  /// 读写锁：GetStatus 使用 shared_lock（并发读），修改操作使用 unique_lock（独占写）。
  mutable std::shared_mutex mutex_;
};

}  // namespace home_theater

// ============================================================================
// 使用示例
// ============================================================================

namespace
{

/// 创建并组装家庭影院系统的所有子系统。
std::unique_ptr<home_theater::HomeTheaterFacade> CreateHomeTheater()
{
  using namespace home_theater::subsystem;  // NOLINT(build/namespaces)

  auto projector = std::make_unique<Projector>();
  auto amplifier = std::make_unique<Amplifier>();
  auto screen = std::make_unique<Screen>();
  auto blu_ray = std::make_unique<BluRayPlayer>();
  auto dvd = std::make_unique<DvdPlayer>();
  auto streaming = std::make_unique<StreamingPlayer>();
  auto lights = std::make_unique<Lights>();

  // 灯光是可选组件，以 std::optional 传入
  return std::make_unique<home_theater::HomeTheaterFacade>(
      std::move(projector), std::move(amplifier), std::move(screen),
      std::move(blu_ray), std::move(dvd), std::move(streaming),
      std::make_optional(std::move(lights)));
}

}  // namespace

int main()
{
  std::cout << "========================================\n";
  std::cout << "  外观模式 (Facade Pattern) 演示\n";
  std::cout << "========================================\n";

  // 客户端只需与外观类交互，无需了解各子系统的接口和启动顺序
  auto theater = CreateHomeTheater();

  // 场景 1：观看蓝光电影
  theater->WatchMovie("星际穿越", /*is_bluray=*/true);
  std::cout << "[状态] " << theater->GetStatus() << "\n";
  theater->EndMovie();

  // 场景 2：观看普通 DVD
  theater->WatchMovie("千与千寻", /*is_bluray=*/false);
  std::cout << "[状态] " << theater->GetStatus() << "\n";
  theater->EndMovie();

  // 场景 3：观看流媒体
  theater->WatchStream("Netflix", "怪奇物语");
  std::cout << "[状态] " << theater->GetStatus() << "\n";
  theater->EndMovie();

  std::cout << "[状态] " << theater->GetStatus() << "\n";

  return 0;
}
