//
// Copyright (c) 2015-2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef LIBERTY_LIGHTXLIBZTHREAD_H
#define LIBERTY_LIGHTXLIBZTHREAD_H

#include "chainparams.h"
#include "concurrentqueue.h"
#include "genwit.h"
#include "xlibz/accumulators.h"
#include <atomic>
#include <boost/function.hpp>
#include <boost/thread.hpp>

extern CChain chainActive;
// Max amount of computation for a single request
const int COMP_MAX_AMOUNT = 60 * 24 * 60;


/****** Thread ********/

class CLightWorker
{
private:
    concurrentqueue<CGenWit> requestsQueue;
    std::atomic<bool> isWorkerRunning;
    boost::thread threadIns;

public:
    CLightWorker()
    {
        isWorkerRunning = false;
    }

    enum ERROR_CODES {
        NOT_ENOUGH_MINTS = 0,
        NON_DETERMINED = 1
    };

    bool addWitWork(CGenWit wit)
    {
        if (!isWorkerRunning) {
            LogPrintf("%s not running trying to add wit work \n", "xlibz-light-thread");
            return false;
        }
        requestsQueue.push(wit);
        return true;
    }

    void StartLightXlibzThread(boost::thread_group& threadGroup)
    {
        LogPrintf("%s thread start\n", "xlibz-light-thread");
        threadIns = boost::thread(boost::bind(&CLightWorker::ThreadLightXLibzSimplified, this));
    }

    void StopLightXlibzThread()
    {
        threadIns.interrupt();
        LogPrintf("%s thread interrupted\n", "xlibz-light-thread");
    }

private:
    void ThreadLightXLibzSimplified();

    void rejectWork(CGenWit& wit, int blockHeight, uint32_t errorNumber);
};

#endif //LIBERTY_LIGHTXLIBZTHREAD_H
