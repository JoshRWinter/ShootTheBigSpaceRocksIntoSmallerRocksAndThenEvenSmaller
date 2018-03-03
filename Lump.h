#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <array>
#include <exception>

#include <string.h>

#include "stbsrisrates.h"
#include "GameState.h"

namespace lmp
{
	enum class Type : std::uint8_t;

	struct netbuf
	{
		netbuf() : offset(0), size(0) {}
		~netbuf()
		{
			if(size > 0 && offset != size)
				hcf("unread lump objects present on the net buffer: offset = " + std::to_string(offset) + ", size = " + std::to_string(size));
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
		}

		template <typename T> const T *pop()
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
				hcf("buffer overwrite when serializing");

			memcpy(nbuf.raw.data() + nbuf.offset, &subject, len);
			nbuf.offset += len;
			nbuf.size += len;
		}

		template <typename T> void read(T &subject, netbuf &nbuf)
		{
			const unsigned len = sizeof(subject);
			if(nbuf.offset + len > nbuf.raw.size())
				hcf("buffer overread when deserializing");

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

	enum class Type : std::uint8_t
	{
		CLIENT_INFO,
		SERVER_INFO,
		PLAYER,
		ASTEROID,
		REMOVE
	};

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

			write(secret, nbuf);
			write(stepno, nbuf);
			write(bits, nbuf);
			write(angle, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			std::uint8_t bits = 0;
			read(secret, nbuf);
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
		std::int32_t secret;
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

		std::uint32_t stepno;
	};

	struct Player : Lump
	{
		Player() : Lump(Type::PLAYER) {}
		Player(const ::Player &subject)
			: Lump(Type::PLAYER)
		{
			id = subject.id;
			x = subject.x;
			y = subject.y;
			xv = subject.xv;
			yv = subject.yv;
			shooting = subject.shooting;
			health = subject.health;
		}

		void serialize(netbuf &nbuf)
		{
			write(type, nbuf);

			write(id, nbuf);
			write(x, nbuf);
			write(y, nbuf);
			write(xv, nbuf);
			write(yv, nbuf);
			write(shooting, nbuf);
			write(health, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(id, nbuf);
			read(x, nbuf);
			read(y, nbuf);
			read(xv, nbuf);
			read(yv, nbuf);
			read(shooting, nbuf);
			read(health, nbuf);
		};

		std::int8_t id;
		std::int16_t x, y;
		float xv, yv;
		std::uint8_t shooting;
		std::uint8_t health;
	};

	struct Remove : Lump
	{
		Remove() : Lump(Type::REMOVE) {}
		Remove(Type rt, std::int32_t ident)
			: Lump(Type::REMOVE)
			, remove_type(rt)
			, id(ident)
		{}

		void serialize(netbuf &nbuf)
		{
			write(type, nbuf);

			write(remove_type, nbuf);
			write(id, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(remove_type, nbuf);
			read(id, nbuf);
		}

		Type remove_type;
		std::int32_t id;
	};
}

#endif // LUMP_H
