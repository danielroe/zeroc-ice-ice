//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
TestIntfI::ping(optional<PingReplyPrx> reply, const Current&)
{
    try
    {
        reply->reply();
    }
    catch(const Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::sendByteSeq(ByteSeq, optional<PingReplyPrx> reply, const Current&)
{
    try
    {
        reply->reply();
    }
    catch(const Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::pingBiDir(Identity id, const Current& current)
{
    try
    {
        //
        // Ensure sending too much data doesn't cause the UDP connection
        // to be closed.
        //
        try
        {
            ByteSeq seq;
            seq.resize(32 * 1024);
            TestIntfPrx(current.con->createProxy(id))->sendByteSeq(seq, nullopt);
        }
        catch(const DatagramLimitException&)
        {
            // Expected.
        }

        //
        // Send the reply through the incoming connection.
        //
        PingReplyPrx(current.con->createProxy(id))->replyAsync();
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        assert(false);
    }
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
