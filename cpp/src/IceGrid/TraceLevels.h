//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_TRACE_LEVELS_H
#define ICE_GRID_TRACE_LEVELS_H

#include <Ice/CommunicatorF.h>
#include <Ice/Logger.h>

namespace IceGrid
{
    class TraceLevels
    {
    public:
        TraceLevels(const Ice::CommunicatorPtr&, const std::string&);

        const int admin;
        const std::string adminCat;

        const int application;
        const std::string applicationCat;

        const int node;
        const std::string nodeCat;

        const int replica;
        const std::string replicaCat;

        const int server;
        const std::string serverCat;

        const int adapter;
        const std::string adapterCat;

        const int object;
        const std::string objectCat;

        const int activator;
        const std::string activatorCat;

        const int patch;
        const std::string patchCat;

        const int locator;
        const std::string locatorCat;

        const int session;
        const std::string sessionCat;

        const int discovery;
        const std::string discoveryCat;

        const Ice::LoggerPtr logger;
    };

} // End namespace IceGrid

#endif
