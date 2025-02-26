#include "sticky_engine.h"
#include "console.h"
#include "easy_socket.h"
#include "ioi.h"
#include "sticky_socket.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>

StickyEngine::StickyEngine(const IoIntf& useIo)
    : io(useIo)
{
}

StickyEngine::~StickyEngine()
{
    for (auto& skt : connections)
    {
        skt->disconnect();
    }
}

void StickyEngine::rebuild_poll_params()
{
    // TODO: someday call this only when sockets are reconnected
    responses.clear();
    std::ranges::transform(
        connections, std::back_inserter(responses),
        [](const auto& skt)
    {
        return pollfd {
            .fd = skt->getDescriptor(),
            .events = POLLIN | POLLPRI | POLLOUT,
            .revents = 0,
        };
    }
    );
}

int StickyEngine::poll(int duration)
{
    for (auto& skt : connections)
    {
        if (skt->getState() == EasySocketIntf::ConnectionState::Disconnected)
        {
            skt->reconnect();
        }
    }

    rebuild_poll_params();
    int events = io.poll(responses.data(), responses.size(), duration);
    if (events > 0)
    {
        for (size_t i = 0; i < responses.size(); i++)
        {
            const auto& response = responses.at(i);
            auto& skt = connections.at(i);
            if (skt->getState() != EasySocketIntf::ConnectionState::Disconnected)
            {
                if (!skt->eval(response))
                {
                    skt->step();
                }
            }
        }
    }
    else if (events < 0)
    {
        console::error("Polling error: {}.", events);
    }
    return events;
}
