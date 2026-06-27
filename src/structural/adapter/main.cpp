/**
 * Adapter Pattern (适配器模式)
 *
 * Intent: Convert the interface of a class into another interface clients expect.
 *         Adapter lets classes work together that couldn't otherwise because of
 *         incompatible interfaces.
 *
 * C++17 features used:
 *   - std::variant / std::visit for type-safe polymorphic dispatch
 *   - std::optional for expressing fallible results
 *   - std::string_view for zero-copy string parameters
 *   - if constexpr for compile-time conditional logic
 *   - structured bindings for clean tuple unpacking
 *   - inline constexpr for ODR-safe global constants
 */

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// ============================================================================
// Pattern Core: Target, Adaptees, and Adapters
// ============================================================================

/// Global constants defining playback parameters.
inline constexpr int k_default_volume = 80;
inline constexpr int k_max_volume = 100;

/// The Target interface that the client expects.
/// In C++17, we define it as a concrete class (not abstract) so it can be
/// stored inside std::variant without slicing issues.
class MediaPlayer
{
 public:
  virtual ~MediaPlayer() = default;

  virtual void Play(std::string_view filename) = 0;
  virtual std::string GetFormat() const = 0;
};

// ---------------------------------------------------------------------------
// Legacy Adaptees — classes with incompatible interfaces
// ---------------------------------------------------------------------------

/// A legacy MP4 player with a non-standard interface.
class LegacyMp4Player
{
 public:
  /// Plays an MP4 file at the given volume. Returns true on success.
  bool PlayOldFormat(const std::string& filepath, int volume) const
  {
    std::cout << "  [LegacyMp4Player] Playing MP4: \"" << filepath
              << "\" at volume " << volume << "\n";
    return !filepath.empty();
  }

  const char* GetCodecName() const
  {
    return "H.264";
  }
};

/// A legacy VLC player with a completely different interface.
class LegacyVlcPlayer
{
 public:
  /// Opens a media file through the VLC engine. Returns a result tuple.
  std::tuple<bool, std::string> OpenMedia(const char* path) const
  {
    std::cout << "  [LegacyVlcPlayer] VLC opening: \"" << path << "\"\n";
    if (path == nullptr || std::string_view(path).empty())
    {
      return {false, "Empty path"};
    }
    return {true, "OK"};
  }

  /// Returns the VLC engine version string.
  std::string EngineVersion() const
  {
    return "VLC 3.0.20";
  }
};

/// A legacy WAV player that only accepts raw buffer pointers.
class LegacyWavPlayer
{
 public:
  /// Plays WAV data from a buffer. Returns the number of samples played.
  int PlayBuffer([[maybe_unused]] const char* data, size_t length,
                 int sample_rate) const
  {
    std::cout << "  [LegacyWavPlayer] Playing WAV buffer (" << length
              << " bytes) at " << sample_rate << " Hz\n";
    return static_cast<int>(length / sizeof(float));
  }
};

// ---------------------------------------------------------------------------
// Adapters — wrap legacy classes and expose the Target interface
// ---------------------------------------------------------------------------

/// Adapter for LegacyMp4Player.
class Mp4Adapter : public MediaPlayer
{
 public:
  explicit Mp4Adapter(std::shared_ptr<LegacyMp4Player> player)
      : player_(std::move(player))
  {
  }

  void Play(std::string_view filename) override
  {
    std::string name(filename);  // Legacy API requires std::string
    std::cout << "[Mp4Adapter] Adapting call...\n";
    bool ok = player_->PlayOldFormat(name, k_default_volume);
    if (!ok)
    {
      std::cout << "  -> Playback failed.\n";
    }
  }

  std::string GetFormat() const override
  {
    return std::string("MP4 (codec: ") + player_->GetCodecName() + ")";
  }

 private:
  std::shared_ptr<LegacyMp4Player> player_;
};

/// Adapter for LegacyVlcPlayer.
class VlcAdapter : public MediaPlayer
{
 public:
  explicit VlcAdapter(std::shared_ptr<LegacyVlcPlayer> player)
      : player_(std::move(player))
  {
  }

  void Play(std::string_view filename) override
  {
    std::string name(filename);
    std::cout << "[VlcAdapter] Adapting call...\n";
    // Structured binding to unpack the tuple result.
    auto [success, message] = player_->OpenMedia(name.c_str());
    if (!success)
    {
      std::cout << "  -> VLC error: " << message << "\n";
    }
    else
    {
      std::cout << "  -> VLC opened successfully ("
                << player_->EngineVersion() << ")\n";
    }
  }

  std::string GetFormat() const override
  {
    return "VLC native";
  }

 private:
  std::shared_ptr<LegacyVlcPlayer> player_;
};

/// Adapter for LegacyWavPlayer using if constexpr to select behavior.
class WavAdapter : public MediaPlayer
{
 public:
  explicit WavAdapter(std::shared_ptr<LegacyWavPlayer> player)
      : player_(std::move(player))
  {
  }

  void Play(std::string_view filename) override
  {
    std::string name(filename);
    std::cout << "[WavAdapter] Adapting call...\n";
    // Simulate loading a WAV buffer; in real code this would read from disk.
    constexpr size_t k_simulated_buffer_size = 4096;
    std::array<char, k_simulated_buffer_size> buffer{};
    int samples = player_->PlayBuffer(buffer.data(), buffer.size(), 44100);
    std::cout << "  -> Played " << samples << " samples from \"" << name
              << "\"\n";
  }

  std::string GetFormat() const override
  {
    return "WAV (PCM)";
  }

 private:
  std::shared_ptr<LegacyWavPlayer> player_;
};

// ============================================================================
// Usage Example
// ============================================================================

/// A helper that demonstrates using if constexpr with a template adapter.
/// This shows how compile-time dispatch can tailor adapter behavior per type.
template <typename LegacyType>
std::string DescribeLegacy([[maybe_unused]] const LegacyType& obj)
{
  if constexpr (std::is_same_v<LegacyType, LegacyMp4Player>)
  {
    return std::string("MP4 legacy player (codec: ") + obj.GetCodecName() + ")";
  }
  else if constexpr (std::is_same_v<LegacyType, LegacyVlcPlayer>)
  {
    return "VLC legacy player (" + obj.EngineVersion() + ")";
  }
  else
  {
    return "Unknown legacy player";
  }
}

/// Print a separator line for output clarity.
void PrintSection(std::string_view title)
{
  std::cout << "\n===== " << title << " =====\n";
}

int main()
{
  // Create legacy player instances (shared_ptr so adapters can share ownership).
  auto mp4_player = std::make_shared<LegacyMp4Player>();
  auto vlc_player = std::make_shared<LegacyVlcPlayer>();
  auto wav_player = std::make_shared<LegacyWavPlayer>();

  // Wrap them in adapters.
  auto mp4_adapter = std::make_shared<Mp4Adapter>(mp4_player);
  auto vlc_adapter = std::make_shared<VlcAdapter>(vlc_player);
  auto wav_adapter = std::make_shared<WavAdapter>(wav_player);

  // -----------------------------------------------------------------------
  // 1. Client code uses the unified MediaPlayer interface via std::variant.
  // -----------------------------------------------------------------------
  PrintSection("Playing through adapters (std::variant dispatch)");

  // Store all adapters in a std::variant — no raw pointers, no virtual table
  // lookup at the variant level (the variant itself holds the concrete type).
  using AnyPlayer = std::variant<
      std::shared_ptr<Mp4Adapter>,
      std::shared_ptr<VlcAdapter>,
      std::shared_ptr<WavAdapter>>;

  std::vector<AnyPlayer> players;
  players.emplace_back(mp4_adapter);
  players.emplace_back(vlc_adapter);
  players.emplace_back(wav_adapter);

  for (const auto& player_variant : players)
  {
    // std::visit dispatches to the correct overload based on the held type.
    std::visit(
        [](const auto& player)
        {
          std::cout << "Format: " << player->GetFormat() << "\n";
          player->Play("demo_media");
          std::cout << "\n";
        },
        player_variant);
  }

  // -----------------------------------------------------------------------
  // 2. Demonstrate if constexpr with template-based compile-time dispatch.
  // -----------------------------------------------------------------------
  PrintSection("Compile-time description via if constexpr");

  std::cout << DescribeLegacy(*mp4_player) << "\n";
  std::cout << DescribeLegacy(*vlc_player) << "\n";

  // -----------------------------------------------------------------------
  // 3. Demonstrate std::optional return from a wrapped adapter call.
  // -----------------------------------------------------------------------
  PrintSection("Fallible playback with std::optional");

  auto TryPlay = [](std::shared_ptr<MediaPlayer> p,
                     std::string_view file) -> std::optional<std::string>
  {
    // In a real system, Play() could throw or return an error code.
    // Here we wrap it to return the format on success, nullopt on empty file.
    if (file.empty())
    {
      return std::nullopt;
    }
    p->Play(file);
    return p->GetFormat();
  };

  if (auto result = TryPlay(mp4_adapter, "action_movie.mp4"); result.has_value())
  {
    std::cout << "  Success! Played format: " << result.value() << "\n";
  }

  if (auto result = TryPlay(vlc_adapter, ""); !result.has_value())
  {
    std::cout << "  Expected failure: empty filename returned nullopt.\n";
  }

  // -----------------------------------------------------------------------
  // 4. Structured binding with a tuple return (simulating a legacy query).
  // -----------------------------------------------------------------------
  PrintSection("Structured bindings from legacy tuple return");

  auto QueryStatus = [&vlc_player]() -> std::tuple<std::string, bool>
  {
    return {vlc_player->EngineVersion(), true};
  };

  auto [version, ready] = QueryStatus();
  std::cout << "Engine: " << version << ", Ready: "
            << (ready ? "yes" : "no") << "\n";

  std::cout << "\nAll adapter demonstrations complete.\n";
  return 0;
}
