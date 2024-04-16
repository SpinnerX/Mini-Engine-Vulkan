#include <Mini-GameEngine/UUID.h>
#include <random>
#include <unordered_map>

namespace MiniGameEngine{
    static std::random_device _randDevice;
    static std::mt19937 _engine(_randDevice());
    static std::uniform_int_distribution<uint64_t> _uniformDistribution;

    UUID::UUID() : uuid(_uniformDistribution(_engine)) {}

    UUID::UUID(uint64_t other_uuid) : uuid(other_uuid) {}

};