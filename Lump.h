#ifndef LUMP_H
#define LUMP_H

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
				hcf("unread lump objects present on the net buffer: offset = %d, size = %d", offset, size);
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

		template <typename T> void push(const T &lump)
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
		virtual void serialize(netbuf&) const = 0;
		virtual void deserialize(netbuf&) = 0;

	protected:
		template <typename T> void write(T subject, netbuf &nbuf) const
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
		SHIP,
		REMOVE
	};

	struct ClientInfo : Lump
	{
		ClientInfo() : Lump(Type::CLIENT_INFO) {}

		void serialize(netbuf &nbuf) const
		{
			std::uint8_t bits = 0;
			bits |= fire << 0;
			bits |= paused << 1;

			std::int8_t int_x = x * 100, int_y = y * 100;

			write(type, nbuf);

			write(secret, nbuf);
			write(stepno, nbuf);
			write(int_x, nbuf);
			write(int_y, nbuf);
			write(bits, nbuf);
			write(angle, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			std::uint8_t bits = 0;

			std::int8_t int_x, int_y;

			read(secret, nbuf);
			read(stepno, nbuf);
			read(int_x, nbuf);
			read(int_y, nbuf);
			read(bits, nbuf);
			read(angle, nbuf);

			fire = (bits >> 0) & 1;
			paused = (bits >> 1) & 1;
			x = int_x / 100.0;
			y = int_y / 100.0;
		}

		std::uint32_t stepno;
		std::int32_t secret;
		float x;
		float y;
		std::uint8_t fire, paused;
		float angle;
	};

	struct ServerInfo : Lump
	{
		ServerInfo() : Lump(Type::SERVER_INFO) {}

		void serialize(netbuf &nbuf) const
		{
			std::uint8_t normal_win = !!win;
			std::uint8_t normal_pause = !!paused;

			std::uint32_t win_and_stepno = normal_win << 31;
			win_and_stepno |= stepno;

			std::uint8_t pause_and_repair = normal_pause << 7;
			pause_and_repair |= repair;

			write(type, nbuf);

			write(win_and_stepno, nbuf);
			write(my_id, nbuf);
			write(pause_and_repair, nbuf);
			write(score, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			std::uint32_t win_and_stepno;
			std::uint8_t pause_and_repair;

			read(win_and_stepno, nbuf);
			read(my_id, nbuf);
			read(pause_and_repair, nbuf);
			read(score, nbuf);

			paused = (pause_and_repair & 128) == 128;
			repair = pause_and_repair & 127;

			win = ((win_and_stepno >> 31) & 1) == 1;
			stepno = win_and_stepno & 2147483647;
		}

		std::uint32_t stepno;
		std::uint8_t my_id;
		std::uint8_t repair;
		std::uint8_t paused;
		std::uint8_t win;
		std::int32_t score;
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
			rot = subject.rot;
			shooting = subject.shooting;
			health = subject.health;
		}

		void serialize(netbuf &nbuf) const
		{
			const float pi = 3.1415926;

			float normal_rot = rot;
			while(normal_rot < 0.0f)
				normal_rot += 2 * pi;
			while(normal_rot >= 2 * pi)
				normal_rot -= 2 * pi;
			std::uint16_t angle = normal_rot * (180.0 / 3.1415926);

			write(type, nbuf);

			write(id, nbuf);
			write(x, nbuf);
			write(y, nbuf);
			write(xv, nbuf);
			write(yv, nbuf);
			write(angle, nbuf);
			write(shooting, nbuf);
			write(health, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			std::uint16_t angle;

			read(id, nbuf);
			read(x, nbuf);
			read(y, nbuf);
			read(xv, nbuf);
			read(yv, nbuf);
			read(angle, nbuf);
			read(shooting, nbuf);
			read(health, nbuf);

			rot = angle * (3.1415926 / 180.0);
		};

		std::int8_t id;
		std::int16_t x, y;
		float xv, yv;
		float rot;
		std::uint8_t shooting;
		std::int8_t health;
	};

	struct Asteroid : Lump
	{
		Asteroid() : Lump(Type::ASTEROID) {}
		Asteroid(const ::Asteroid &subject)
			: Lump(Type::ASTEROID)
		{
			aster_type = subject.type;
			id = subject.id;
			x = subject.x - subject.xv;
			y = subject.y - subject.yv;
			xv = subject.xv;
			yv = subject.yv;
		}

		void serialize(netbuf &nbuf) const
		{
			write(type, nbuf);

			write(aster_type, nbuf);
			write(id, nbuf);
			write(x, nbuf);
			write(y, nbuf);
			write(xv, nbuf);
			write(yv, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(aster_type, nbuf);
			read(id, nbuf);
			read(x, nbuf);
			read(y, nbuf);
			read(xv, nbuf);
			read(yv, nbuf);
		}

		AsteroidType aster_type;
		std::int32_t id;
		std::int16_t x;
		std::int16_t y;
		float xv;
		float yv;
	};

	struct Ship : Lump
	{
		Ship() : Lump(Type::SHIP) {}
		Ship(const ::Ship &subject)
			: Lump(Type::SHIP)
		{
			id = subject.id;
			x = subject.x;
			y = subject.y;
			xv = subject.xv;
			yv = subject.yv;
			health = subject.health;
		}

		void serialize(netbuf &nbuf) const
		{
			write(type, nbuf);

			write(id, nbuf);
			write(x, nbuf);
			write(y, nbuf);
			write(xv, nbuf);
			write(yv, nbuf);
			write(health, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(id, nbuf);
			read(x, nbuf);
			read(y, nbuf);
			read(xv, nbuf);
			read(yv, nbuf);
			read(health, nbuf);
		}

		std::int32_t id;
		std::int16_t x;
		std::int16_t y;
		float xv;
		float yv;
		std::int8_t health;
	};

	struct Remove : Lump
	{
		Remove() : Lump(Type::REMOVE), ref(Entity::Type(), 0) {}
		Remove(Entity::Reference er)
			: Lump(Type::REMOVE)
			, ref(er)
		{}

		void serialize(netbuf &nbuf) const
		{
			write(type, nbuf);

			write(ref.type, nbuf);
			write(ref.id, nbuf);
		}

		void deserialize(netbuf &nbuf)
		{
			read(ref.type, nbuf);
			read(ref.id, nbuf);
		}

		Entity::Reference ref;
	};
}

#endif // LUMP_H
