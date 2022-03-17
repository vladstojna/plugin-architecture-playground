#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

namespace midi {

class midiin_attributes;

struct driver_version_t {
  std::byte major;
  std::byte minor;
};

struct midiin_descriptor_t {
  uint32_t port;
  uint16_t manufacturer_id;
  uint16_t product_id;
  driver_version_t driver_version;
  std::string name;

  explicit midiin_descriptor_t(uint32_t);
};

uint32_t midiin_port_count() noexcept;

std::vector<std::pair<uint32_t, midiin_descriptor_t>> midiin_descriptors();

std::vector<std::pair<uint32_t, midiin_descriptor_t>>
midiin_descriptors(std::error_code &);

bool operator==(const driver_version_t &, const driver_version_t &) noexcept;

bool operator==(const midiin_descriptor_t &,
                const midiin_descriptor_t &) noexcept;

enum class start_behaviour : uint32_t {
  start,
  defer,
};

class midiin {
public:
  midiin(midiin_descriptor_t, midiin_attributes,
         start_behaviour = start_behaviour::start);

  midiin(midiin &&) noexcept;
  midiin &operator=(midiin &&) noexcept;

  ~midiin();

  const midiin_descriptor_t &descriptor() const noexcept;

  void start();
  bool start(std::error_code &) noexcept;

  void stop();
  bool stop(std::error_code &) noexcept;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

} // namespace midi
