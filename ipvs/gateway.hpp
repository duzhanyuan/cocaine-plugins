/*
    Copyright (c) 2011-2013 Andrey Sibiryov <me@kobology.ru>
    Copyright (c) 2011-2013 Other contributors as noted in the AUTHORS file.

    This file is part of Cocaine.

    Cocaine is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Cocaine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COCAINE_IPVS_GATEWAY_HPP
#define COCAINE_IPVS_GATEWAY_HPP

#include "cocaine/api/gateway.hpp"

#include <queue>

extern "C" {
    #include "libipvs-1.25/libipvs.h"
}

namespace cocaine { namespace gateway {

class ipvs_t:
    public api::gateway_t
{
    public:
        ipvs_t(context_t& context, const std::string& name, const Json::Value& args);

        virtual
       ~ipvs_t();

        virtual
        api::resolve_result_type
        resolve(const std::string& name) const;

        virtual
        void
        consume(const std::string& uuid, api::synchronize_result_type dump);

        virtual
        void
        prune(const std::string& uuid);

    private:
        void
        add_backend(const std::string& name, const std::string& uuid, ipvs_dest_t backend);

        void
        pop_backend(const std::string& name, const std::string& uuid);

    private:
        context_t& m_context;
        std::unique_ptr<logging::log_t> m_log;

        const std::string m_default_scheduler;
        const unsigned    m_default_weight;

        // Ports available for allocation to virtual services.
        std::priority_queue<uint16_t, std::vector<uint16_t>, std::greater<uint16_t>> m_ports;

        struct service_info_t {
            unsigned int version;

            // NOTE: I hope it will be same across all the nodes in a group.
            std::tuple_element<2, api::resolve_result_type>::type map;
        };

        typedef std::map<std::string, service_info_t> service_info_map_t;

        // Keeps track of service versions and mappings.
        service_info_map_t m_service_info;

        struct remote_service_t {
            ipvs_service_t handle;

            // Virtual service endpoint.
            std::tuple<std::string, uint16_t> endpoint;

            // Backend UUID -> Destination mapping.
            std::map<std::string, ipvs_dest_t> backends;
        };

        typedef std::map<std::string, remote_service_t> remote_service_map_t;

        // Keeps track of IPVS configuration.
        remote_service_map_t m_remote_services;

        typedef std::map<std::string, api::synchronize_result_type> history_map_t;

        // Keeps track of last update from every node to effectively drop stale backends.
        history_map_t m_history;
};

}} // namespace cocaine::gateway

#endif