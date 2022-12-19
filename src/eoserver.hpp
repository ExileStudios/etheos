
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOSERVER_HPP_INCLUDED
#define EOSERVER_HPP_INCLUDED

#include "fwd/eoserver.hpp"

#include "fwd/config.hpp"
#include "fwd/database.hpp"
#include "fwd/eoclient.hpp"
#include "fwd/sln.hpp"
#include "fwd/timer.hpp"
#include "fwd/world.hpp"

#include "socket.hpp"

#include <array>
#include <string>
#include <unordered_map>

void server_ping_all(void *server_void);
void server_pump_queue(void *server_void);

struct ConnectionLogEntry
{
	double last_connection_time = 0.0;
	double last_rejection_time = 0.0;
	int rejections = 0;
};

/**
 * A server which accepts connections and creates EOClient instances from them
 */
class EOServer : public Server
{
	private:
		std::unordered_map<IPAddress, ConnectionLogEntry> connection_log;
		void Initialize(std::shared_ptr<DatabaseFactory> databaseFactory, const Config &eoserv_config, const Config &admin_config);

		TimeEvent* ping_timer = nullptr;

	protected:
		virtual Client *ClientFactory(const Socket &);

	public:
		World *world;
		double start;
		SLN *sln;

		bool QuietConnectionErrors = false;
		double HangupDelay = 10.0;

		void UpdateConfig();

		EOServer(IPAddress addr, unsigned short port, std::shared_ptr<DatabaseFactory> databaseFactory, const Config &eoserv_config, const Config &admin_config) : Server(addr, port)
		{
			this->Initialize(databaseFactory, eoserv_config, admin_config);
		}

		void Tick();

		void RecordClientRejection(const IPAddress& ip, const char* reason);
		void CleanupConnectionLog();

		~EOServer();
};

#endif // EOSERVER_HPP_INCLUDED
