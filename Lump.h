#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <array>
#include <exception>

#include <string.h>

#include "stbsrisrates.h"

namespace lmp
{
	class LumpException : std::exception
	{
	public:
		LumpException(const std::string &msg) : message(msg) {}
		virtual const char *what() const noexcept { return message.c_str(); }

	private:
		const std::string message;
	};

	enum class Type : std::uint8_t
	{
		CLIENT_INFO,
		SERVER_INFO,
		PLAYER,
		ASTEROID
	};

	struct netbuf
	{
		netbuf() : offset(0), size(0) {}
		~netbuf()
		{
			if(size > 0 && offset != size)
			{
				log("unread lump objects present on the net buffer: offset = " + std::to_string(offset) + ", size = " + std::to_string(size));
				std::abort();
			}
		}

		netbuf(const netbuf&) = delete;
		void operator=(const netbuf&) = delete;

		void reset()
		{
			size = 0;
			offset = 0;
		}

		static bool get(netbuf &buf, net::udp_server &udp, net::udp_id &id)
		{
			buf.reset();
			buf.size = udp.recv(buf.raw.data(), buf.raw.size(), id);
			return buf.size > 0;
		}

		static bool get(netbuf &buf, net::udp &udp)
		{
			buf.reset();
			buf.size = udp.recv(buf.raw.data(), buf.raw.size());
			return buf.size > 0;
		}

		template <typename T> void push(T &lump)
		{
			lump.serialize(*this);
			size = offset;
		}

		template <typename T> T *pop()
		{
			if(offset >= size)
				return NULL;

			static T static_lump;

			// read type
			Type type;
			memcpy(&type, raw.data() + offset, sizeof(type));

			if(type != static_lump.type)
				return NULL;

			offset += sizeof(type);
			static_lump.deserialize(*this);

			return &static_lump;
		}

		std::array<std::uint8_t, MAX_DATAGRAM_SIZE> raw;
		unsigned offset;
		unsigned size;
	};

	struct Lump
	{
	public:
		const Type type;

		Lump(Type tp) : type(tp) {}
		virtual void serialize(netbuf&) = 0;
		virtual void deserialize(netbuf&) = 0;

	protected:
		template <typename T> void write(T subject, netbuf &nbuf)
		{
			const unsigned len = sizeof(subject);
			if(nbuf.offset + len > nbuf.raw.size())
				throw LumpException("buffer overwrite when serializing");

			memcpy(nbuf.raw.data() + nbuf.offset, &subject, len);
			nbuf.offset += len;
		}

		template <typename T> void read(T &subject, netbuf &nbuf)
		{
			const unsigned len = sizeof(subject);
			if(nbuf.offset + len > nbuf.raw.size())
				throw LumpException("buffer overread when deserializing");

			memcpy(&subject, nbuf.raw.data() + nbuf.offset, len);
			nbuf.offset += len;
		}
	};

	// *********************
	// *********************
	// APPLICATION
	//  SPECIFIC
	//    LUMP
	//   OBJECTS
	// *********************
	// *********************

	struct ClientInfo : Lump
	{
		ClientInfo() : Lump(Type::CLIENT_INFO) {}

		void serialize(netbuf &nbuf)
		{
			std::uint8_t bits = 0;
			bits |= up << 7;
			bits |= down << 6;
			bits |= left << 5;
			bits |= right << 4;
			bits |= fire << 3;
			bits |= pause << 2;

			write(type, nbuf);

			write(stepno, nbuf);
			write(bits, nbuf);
			write(angle, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			std::uint8_t bits = 0;
			read(stepno, nbuf);
			read(bits, nbuf);
			read(angle, nbuf);

			up = (bits >> 7) & 1;
			down = (bits >> 6) & 1;
			left = (bits >> 5) & 1;
			right = (bits >> 4) & 1;
			fire = (bits >> 3) & 1;
			pause = (bits >> 2) & 1;
		}

		int up, down, left, right, fire, pause;
		std::uint16_t angle;
		std::uint32_t stepno;
	};

	struct ServerInfo : Lump
	{
		ServerInfo() : Lump(Type::SERVER_INFO) {}

		void serialize(netbuf &nbuf)
		{
			write(type, nbuf);

			write(stepno, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(stepno, nbuf);
		}

		uint32_t stepno;
	};

	struct Player : Lump
	{
		Player() : Lump(Type::PLAYER) {}

		void serialize(netbuf &nbuf)
		{
			write(type, nbuf);

			write(x, nbuf);
			write(y, nbuf);
			write(xv, nbuf);
			write(yv, nbuf);
			write(shooting, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(x, nbuf);
			read(y, nbuf);
			read(xv, nbuf);
			read(yv, nbuf);
			read(shooting, nbuf);
		};

		int16_t x, y;
		float xv, yv;
		uint8_t shooting;
	};
}

#endif // LUMP_H
