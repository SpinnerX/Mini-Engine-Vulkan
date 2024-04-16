#pragma once
#include <cstdint>

namespace MiniGameEngine{
    /**
     * @name UUID
     * @note Used for giving our objects (reppresented as entities/actors) a unique ID
     * @note For identifying which object we want to modify through our quick UI.
    */
    class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return uuid; }
	private:
		uint64_t uuid;
	};
};

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<MiniGameEngine::UUID>
	{
		std::size_t operator()(const MiniGameEngine::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}