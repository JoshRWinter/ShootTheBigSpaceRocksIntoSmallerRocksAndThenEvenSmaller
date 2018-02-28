#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <array>
#include <exception>

#include <string.h>

#include "stbsrisrates.h"

namespace lmp
{
	typedef std::array<std::uint8_t, MAX_DATAGRAM_SIZE> rawbuf;

	class LumpException : public std::exception
	{
	public:
		virtual const char *what() const noexcept
		{
			return "buffer overrun when serializing or deserializing";
		}
	};

	enum class Type : std::uint8_t
	{
		CLIENT_INFO,
		SERVER_INFO,
		ASTEROID,
		PLAYER
	};

	struct Lump
	{
	public:
		const Type type;
		rawbuf &buffer;
		const unsigned offset;
		unsigned size;

		Lump(Type tp, rawbuf &b, unsigned offs) : type(tp), buffer(b), offset(offs), size(0) {}
		virtual void serialize() = 0;
		virtual void deserialize() = 0;

	protected:
		template <typename T> int write(T subject)
		{
			const unsigned len = sizeof(subject);
			if(offset + len > buffer.size())
				throw LumpException();

			memcpy(buffer.data() + offset + size, &subject, len);
			size += len;

			return len;
		}

		template <typename T> int read(T &subject)
		{
			const unsigned len = sizeof(subject);
			if(offset + len > buffer.size())
				throw LumpException();

			memcpy(&subject, buffer.data() + offset + size, len);
			size += len;

			return len;
		}
	};

	struct ClientInfo : Lump
	{
		ClientInfo(rawbuf &buf, unsigned offset) : Lump(Type::CLIENT_INFO, buf, offset) {}

		void serialize()
		{
			std::uint8_t bits = 0;
			bits |= up << 8;
			bits |= down << 7;
			bits |= left << 6;
			bits |= right << 5;
			bits |= fire << 4;
			bits |= pause << 3;

			write(type);

			write(stepno);
			write(bits);
			write(angle);
		}

		void deserialize()
		{
			std::uint8_t bits = 0;
			read(stepno);
			read(bits);
			read(angle);

			up = (bits >> 8) & 1;
			down = (bits >> 7) & 1;
			left = (bits >> 6) & 1;
			right = (bits >> 5) & 1;
			fire = (bits >> 4) & 1;
			pause = (bits >> 3) & 1;
		}

		std::uint8_t up, down, left, right, fire, pause;
		std::uint16_t angle;
		std::uint32_t stepno;
	};

	struct ServerInfo : Lump
	{
		ServerInfo(rawbuf &b, unsigned offs) : Lump(Type::SERVER_INFO, b, offs) {}

		void serialize()
		{
			write(type);

			write(stepno);
		}

		void deserialize()
		{
			read(stepno);
		}

		uint32_t stepno;
	};

	struct Player : Lump
	{
		Player(rawbuf &buf, unsigned offset) : Lump(Type::PLAYER, buf, offset) {}

		void serialize()
		{
			write(type);

			write(x);
			write(y);
			write(xv);
			write(yv);
			write(shooting);
		}

		void deserialize()
		{
			read(x);
			read(y);
			read(xv);
			read(yv);
			read(shooting);
		};

		int16_t x, y;
		float xv, yv;
		uint8_t shooting;
	};
}

#endif // LUMP_H
