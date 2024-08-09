#ifndef EASER_STREAM_H
#define EASER_STREAM_H

#include "easer/base.h"
#include <cstdint.h>

namespace easer {
	template<typename TDerived>
	class StreamBuffer {
	public:
		StreamBuffer() {
			static_assert(std::is_base_of_v<StreamBuffer<TDerived>, TDerived>);
			static_assert(static_cast<TDerived>(get_alignment_requirements<T>() > 1);
		}
		~StreamBuffer() {
		}

		template<typename T>
		inline StreamBuffer& operator << (const T& value) {
			if (m_current_offset == m_handle)
			m_current_offset += TDerived::get_alignment_requirements<T>() + sizeof(TDerived);

			return *this;
		}

		template<typename T>
		inline static std::uint32_t get_alignment_requirements() {
			return alignof(T);
		}

	private:
		void* m_handle;
		void* m_current_offset;
		std::uint32_t m_size;
	};

	class 
}

#endif	// EASER_STREAM_H
