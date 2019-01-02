// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2015-2018 The PIVX Developers 
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "db.h"
#include "init.h"
#include "main.h"
#include "masternode-budget.h"
#include "masternode-payments.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "rpc/server.h"
#include "utilmoneystr.h"

#include <univalue.h>

#include <boost/tokenizer.hpp>
#include <fstream>

UniValue getpoolinfo(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getpoolinfo\n"
            "\nReturns anonymous pool-related information\n"

            "\nResult:\n"
            "{\n"
            "  \"current\": \"addr\",    (string) Liberty address of current masternode\n"
            "  \"state\": xxxx,        (string) unknown\n"
            "  \"entries\": xxxx,      (numeric) Number of entries\n"
            "  \"accepted\": xxxx,     (numeric) Number of entries accepted\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getpoolinfo", "") + HelpExampleRpc("getpoolinfo", ""));

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("current_masternode", mnodeman.GetCurrentMasterNode()->addr.ToString()));
    obj.push_back(Pair("state", obfuScationPool.GetState()));
    obj.push_back(Pair("entries", obfuScationPool.GetEntriesCount()));
    obj.push_back(Pair("entries_accepted", obfuScationPool.GetCountEntriesAccepted()));
    return obj;
}

UniValue listmasternodes(const UniValue& params, bool fHelp)
{
    std::string strFilter = "";

    if (params.size() == 1) strFilter = params[0].get_str();

    if (fHelp || (params.size() > 1))
        throw runtime_error(
            "listmasternodes ( \"filter\" )\n"
            "\nget a ranked list of masternodes\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) filter search text. partial match by txhash, status, or addr.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"rank\": n,           (numeric) masternode Rank (or 0 if not enabled)\n"
            "    \"txhash\": \"hash\",    (string) collateral transaction hash\n"
            "    \"outidx\": n,         (numeric) collateral transaction output index\n"
            "    \"pubkey\": \"key\",   (string) masternode public key used for message broadcasting\n"
            "    \"status\": s,         (string) status (ENABLED/EXPIRED/REMOVE/etc)\n"
            "    \"addr\": \"addr\",      (string) masternode liberty address\n"
            "    \"version\": v,        (numeric) masternode protocol version\n"
            "    \"lastseen\": ttt,     (numeric) the time in seconds since epoch (Jan 1 1970 GMT) of the last seen\n"
            "    \"activetime\": ttt,   (numeric) the time in seconds since epoch (Jan 1 1970 GMT) masternode has been active\n"
            "    \"lastpaid\": ttt,     (numeric) the time in seconds since epoch (Jan 1 1970 GMT) masternode was last paid\n"
            "    \"tier\":xxxx,         (numeric) masternode tier\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nexamples:\n" +
            HelpExampleCli("listmasternodes", "") + HelpExampleRpc("listmasternodes", ""));

    UniValue ret(UniValue::VARR);
    int nHeight;
    {
        LOCK(cs_main);
        CBlockIndex* pindex = chainActive.Tip();
        if(!pindex) return 0;
        nHeight = pindex->nHeight;
    }
    std::vector<pair<int, CMasternode> > vMasternodeRanks = mnodeman.GetMasternodeRanks(nHeight);
    BOOST_FOREACH (PAIRTYPE(int, CMasternode) & s, vMasternodeRanks) {
        UniValue obj(UniValue::VOBJ);
        std::string strVin = s.second.vin.prevout.ToStringShort();
        std::string strTxHash = s.second.vin.prevout.hash.ToString();
        uint32_t oIdx = s.second.vin.prevout.n;

        CMasternode* mn = mnodeman.Find(s.second.vin);

        if (mn != NULL) {
            if (strFilter != "" && strTxHash.find(strFilter) == string::npos &&
                mn->Status().find(strFilter) == string::npos &&
                CBitcoinAddress(mn->pubKeyCollateralAddress.GetID()).ToString().find(strFilter) == string::npos) continue;

            std::string strStatus = mn->Status();
            std::string strHost;
            int port;
            SplitHostPort(mn->addr.ToString(), port, strHost);
            CNetAddr node = CNetAddr(strHost, false);
            std::string strNetwork = GetNetworkName(node.GetNetwork());
            std::pair<tier, CAmount> tierInfo = mn->GetMasternodeTierInfo();

            obj.push_back(Pair("rank", (strStatus == "ENABLED" ? s.first : 0)));
            obj.push_back(Pair("network", strNetwork));
            obj.push_back(Pair("txhash", strTxHash));
            obj.push_back(Pair("outidx", (uint64_t)oIdx));
            obj.push_back(Pair("pubkey", HexStr(mn->pubKeyMasternode)));
            obj.push_back(Pair("status", strStatus));
            obj.push_back(Pair("addr", CBitcoinAddress(mn->pubKeyCollateralAddress.GetID()).ToString()));
            obj.push_back(Pair("version", mn->protocolVersion));
            obj.push_back(Pair("lastseen", (int64_t)mn->lastPing.sigTime));
            obj.push_back(Pair("activetime", (int64_t)(mn->lastPing.sigTime - mn->sigTime)));
            obj.push_back(Pair("lastpaid", (int64_t)mn->GetLastPaid()));
            obj.push_back(Pair("tier", tierInfo.first));

            ret.push_back(obj);
        }
    }

    return ret;
}

UniValue masternodeconnect(const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 1))
        throw runtime_error(
            "masternodeconnect \"address\"\n"
            "\nattempts to connect to specified masternode address\n"

            "\nArguments:\n"
            "1. \"address\"     (string, required) IP or net address to connect to\n"

            "\nexamples:\n" +
            HelpExampleCli("masternodeconnect", "\"192.168.0.6:10417\"") + HelpExampleRpc("masternodeconnect", "\"192.168.0.6:10417\""));

    std::string strAddress = params[0].get_str();

    CService addr = CService(strAddress);

    CNode* pnode = ConnectNode((CAddress)addr, NULL, false);
    if (pnode) {
        pnode->Release();
        return NullUniValue;
    } else {
        throw runtime_error("error connecting\n");
    }
}

UniValue getmasternodecount (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() > 0))
        throw runtime_error(
            "getmasternodecount\n"
            "\nget masternode count values\n"

            "\nresult:\n"
            "{\n"
            "  \"total\": n,        (numeric) total masternodes\n"
            "  \"stable\": n,       (numeric) stable count\n"
            "  \"obfcompat\": n,    (numeric) obfuscation Compatible\n"
            "  \"enabled\": n,      (numeric) enabled masternodes\n"
            "  \"inqueue\": n       (numeric) masternodes in queue\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("getmasternodecount", "") + HelpExampleRpc("getmasternodecount", ""));

    UniValue obj(UniValue::VOBJ);
    int nCount = 0;
    int ipv4 = 0, ipv6 = 0, onion = 0;

    if (chainActive.Tip())
        mnodeman.GetNextMasternodeInQueueForPayment(chainActive.Tip()->nHeight, true, nCount);

    mnodeman.CountNetworks(ActiveProtocol(), ipv4, ipv6, onion);

    obj.push_back(Pair("total", mnodeman.size()));
    obj.push_back(Pair("stable", mnodeman.stable_size()));
    obj.push_back(Pair("obfcompat", mnodeman.CountEnabled(ActiveProtocol())));
    obj.push_back(Pair("enabled", mnodeman.CountEnabled()));
    obj.push_back(Pair("inqueue", nCount));
    obj.push_back(Pair("ipv4", ipv4));
    obj.push_back(Pair("ipv6", ipv6));
    obj.push_back(Pair("onion", onion));

    return obj;
}

UniValue masternodecurrent (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "masternodecurrent\n"
            "\nget current masternode winner\n"

            "\nResult:\n"
            "{\n"
            "  \"protocol\": xxxx,        (numeric) protocol version\n"
            "  \"txhash\": \"xxxx\",      (string) collateral transaction hash\n"
            "  \"pubkey\": \"xxxx\",      (string) masternode public key\n"
            "  \"lastseen\": xxx,       (numeric) time since epoch of last seen\n"
            "  \"activeseconds\": xxx,  (numeric) seconds masternode has been active\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("masternodecurrent", "") + HelpExampleRpc("masternodecurrent", ""));

    CMasternode* winner = mnodeman.GetCurrentMasterNode(1);
    if (winner) {
        UniValue obj(UniValue::VOBJ);

        obj.push_back(Pair("protocol", (int64_t)winner->protocolVersion));
        obj.push_back(Pair("txhash", winner->vin.prevout.hash.ToString()));
        obj.push_back(Pair("pubkey", CBitcoinAddress(winner->pubKeyCollateralAddress.GetID()).ToString()));
        obj.push_back(Pair("lastseen", (winner->lastPing == CMasternodePing()) ? winner->sigTime : (int64_t)winner->lastPing.sigTime));
        obj.push_back(Pair("activeseconds", (winner->lastPing == CMasternodePing()) ? 0 : (int64_t)(winner->lastPing.sigTime - winner->sigTime)));
        return obj;
    }

    throw runtime_error("unknown");
}

UniValue masternodedebug (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "masternodedebug\n"
            "\nprint masternode status\n"

            "\nresult:\n"
            "\"status\"     (string) masternode status message\n"

            "\nexamples:\n" +
            HelpExampleCli("masternodedebug", "") + HelpExampleRpc("masternodedebug", ""));

    if (activeMasternode.status != ACTIVE_MASTERNODE_INITIAL || !masternodeSync.IsSynced())
        return activeMasternode.GetStatus();

    CTxIn vin = CTxIn();
    CPubKey pubkey;
    CKey key;
    if (!activeMasternode.GetMasterNodeVin(vin, pubkey, key))
        throw runtime_error("Missing masternode input, please look at the documentation for instructions on masternode creation\n");
    else
        return activeMasternode.GetStatus();
}

UniValue startmasternode (const UniValue& params, bool fHelp)
{
    std::string strCommand;
    if (params.size() >= 1) {
        strCommand = params[0].get_str();

        // Backwards compatibility with legacy 'masternode' super-command forwarder
        if (strCommand == "start") strCommand = "local";
        if (strCommand == "start-alias") strCommand = "alias";
        if (strCommand == "start-all") strCommand = "all";
        if (strCommand == "start-many") strCommand = "many";
        if (strCommand == "start-missing") strCommand = "missing";
        if (strCommand == "start-disabled") strCommand = "disabled";
    }

    if (fHelp || params.size() < 2 || params.size() > 3 ||
        (params.size() == 2 && (strCommand != "local" && strCommand != "all" && strCommand != "many" && strCommand != "missing" && strCommand != "disabled")) ||
        (params.size() == 3 && strCommand != "alias"))
        throw runtime_error(
            "startmasternode \"local|all|many|missing|disabled|alias\" lockwallet ( \"alias\" )\n"
            "\nattempts to start one or more masternode(s)\n"

            "\narguments:\n"
            "1. set         (string, required) specify which set of masternode(s) to start.\n"
            "2. lockwallet  (boolean, required) lock wallet after completion.\n"
            "3. alias       (string) masternode alias. required if using 'alias' as the set.\n"

            "\nresult: (for 'local' set):\n"
            "\"status\"     (string) masternode status message\n"

            "\nresult: (for other sets):\n"
            "{\n"
            "  \"overall\": \"xxxx\",     (string) overall status message\n"
            "  \"detail\": [\n"
            "    {\n"
            "      \"node\": \"xxxx\",    (string) node name or alias\n"
            "      \"result\": \"xxxx\",  (string) 'success' or 'failed'\n"
            "      \"error\": \"xxxx\"    (string) error message, if failed\n"
            "    }\n"
            "    ,...\n"
            "  ]\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("startmasternode", "\"alias\" \"0\" \"my_mn\"") + HelpExampleRpc("startmasternode", "\"alias\" \"0\" \"my_mn\""));

    bool fLock = (params[1].get_str() == "true" ? true : false);

    EnsureWalletIsUnlocked();

    if (strCommand == "local") {
        if (!fMasterNode) throw runtime_error("you must set masternode=1 in the configuration\n");

        if (activeMasternode.status != ACTIVE_MASTERNODE_STARTED) {
            activeMasternode.status = ACTIVE_MASTERNODE_INITIAL; // TODO: consider better way
            activeMasternode.ManageStatus();
            if (fLock)
                pwalletMain->Lock();
        }

        return activeMasternode.GetStatus();
    }

    if (strCommand == "all" || strCommand == "many" || strCommand == "missing" || strCommand == "disabled") {
        if ((strCommand == "missing" || strCommand == "disabled") &&
            (masternodeSync.RequestedMasternodeAssets <= MASTERNODE_SYNC_LIST ||
                masternodeSync.RequestedMasternodeAssets == MASTERNODE_SYNC_FAILED)) {
            throw runtime_error("you can't use this command until masternode list is synced\n");
        }

        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        BOOST_FOREACH (CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            std::string errorMessage;
            int nIndex;
            if(!mne.castOutputIndex(nIndex))
                continue;
            CTxIn vin = CTxIn(uint256(mne.getTxHash()), uint32_t(nIndex));
            CMasternode* pmn = mnodeman.Find(vin);
            CMasternodeBroadcast mnb;

            if (pmn != NULL) {
                if (strCommand == "missing") continue;
                if (strCommand == "disabled" && pmn->IsEnabled()) continue;
            }

            bool result = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("result", result ? "success" : "failed"));

            if (result) {
                successful++;
                statusObj.push_back(Pair("error", ""));
            } else {
                failed++;
                statusObj.push_back(Pair("error", errorMessage));
            }

            resultsObj.push_back(statusObj);
        }
        if (fLock)
            pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("successfully started %d\n, failed to start %d\n, total %d\n", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "alias") {
        std::string alias = params[2].get_str();

        bool found = false;
        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);
        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH (CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            if (mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                CMasternodeBroadcast mnb;

                bool result = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));

                if (result) {
                    successful++;
                    mnodeman.UpdateMasternodeList(mnb);
                    mnb.Relay();
                } else {
                    failed++;
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if (!found) {
            failed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("error", "could not find alias in config; verify with list-conf."));
        }

        resultsObj.push_back(statusObj);

        if (fLock)
            pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("successfully started %d\n, failed to start %d\n, total %d\n", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }
    return NullUniValue;
}

UniValue createmasternodekey (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "createmasternodekey\n"
            "\ncreate a new masternode private key\n"

            "\nresult:\n"
            "\"key\"    (string) masternode private key\n"

            "\nexamples:\n" +
            HelpExampleCli("createmasternodekey", "") + HelpExampleRpc("createmasternodekey", ""));

    CKey secret;
    secret.MakeNewKey(false);

    return CBitcoinSecret(secret).ToString();
}

UniValue getmasternodeoutputs (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "getmasternodeoutputs\n"
            "\nprint all masternode transaction outputs\n"

            "\nresult:\n"
            "[\n"
            "  {\n"
            "    \"txhash\": \"xxxx\",    (string) output transaction hash\n"
            "    \"outputidx\": n       (numeric) output index number\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nexamples:\n" +
            HelpExampleCli("getmasternodeoutputs", "") + HelpExampleRpc("getmasternodeoutputs", ""));

    // Find possible candidates
    vector<COutput> possibleCoins = activeMasternode.SelectCoinsMasternode();

    UniValue ret(UniValue::VARR);
    BOOST_FOREACH (COutput& out, possibleCoins) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("txhash", out.tx->GetHash().ToString()));
        obj.push_back(Pair("outputidx", out.i));
        ret.push_back(obj);
    }

    return ret;
}

UniValue listmasternodeconf (const UniValue& params, bool fHelp)
{
    std::string strFilter = "";

    if (params.size() == 1) strFilter = params[0].get_str();

    if (fHelp || (params.size() > 1))
        throw runtime_error(
            "listmasternodeconf ( \"filter\" )\n"
            "\nprint masternode.conf in JSON format\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) filter search text. partial match on alias, address, txHash, or status.\n"

            "\nresult:\n"
            "[\n"
            "  {\n"
            "    \"alias\": \"xxxx\",        (string) masternode alias\n"
            "    \"address\": \"xxxx\",      (string) masternode IP address\n"
            "    \"privateKey\": \"xxxx\",   (string) masternode private key\n"
            "    \"txHash\": \"xxxx\",       (string) transaction hash\n"
            "    \"outputIndex\": n,       (numeric) transaction output index\n"
            "    \"status\": \"xxxx\"        (string) masternode status\n"
            "    \"tier\":  \"xxxx\"        (numeric) masternode tier\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nexamples:\n" +
            HelpExampleCli("listmasternodeconf", "") + HelpExampleRpc("listmasternodeconf", ""));

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    mnEntries = masternodeConfig.getEntries();

    UniValue ret(UniValue::VARR);

    BOOST_FOREACH (CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;
        CTxIn vin = CTxIn(uint256(mne.getTxHash()), uint32_t(nIndex));
        CMasternode* pmn = mnodeman.Find(vin);
        std::pair<tier, CAmount> tierInfo = pmn->GetMasternodeTierInfo();
        std::string strStatus = pmn ? pmn->Status() : "MISSING";

        if (strFilter != "" && mne.getAlias().find(strFilter) == string::npos &&
            mne.getIp().find(strFilter) == string::npos &&
            mne.getTxHash().find(strFilter) == string::npos &&
            strStatus.find(strFilter) == string::npos) continue;

        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("alias", mne.getAlias()));
        mnObj.push_back(Pair("address", mne.getIp()));
        mnObj.push_back(Pair("privateKey", mne.getPrivKey()));
        mnObj.push_back(Pair("txHash", mne.getTxHash()));
        mnObj.push_back(Pair("outputIndex", mne.getOutputIndex()));
        mnObj.push_back(Pair("status", strStatus));
        mnObj.push_back(Pair("tier", tierInfo.first));
        ret.push_back(mnObj);
    }

    return ret;
}

UniValue getmasternodestatus (const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() != 0))
        throw runtime_error(
            "getmasternodestatus\n"
            "\nprint masternode status\n"

            "\nresult:\n"
            "{\n"
            "  \"txhash\": \"xxxx\",      (string) collateral transaction hash\n"
            "  \"outputidx\": n,        (numeric) collateral transaction output index number\n"
            "  \"netaddr\": \"xxxx\",     (string) masternode network address\n"
            "  \"addr\": \"xxxx\",        (string) liberty address for masternode payments\n"
            "  \"status\": \"xxxx\",      (string) masternode status\n"
            "  \"tier\":  \"xxxx\",     (numeric) masternode tier\n"
            "  \"message\": \"xxxx\"      (string) masternode status message\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("getmasternodestatus", "") + HelpExampleRpc("getmasternodestatus", ""));

    if (!fMasterNode) throw runtime_error("this is not a masternode");

    CMasternode* pmn = mnodeman.Find(activeMasternode.vin);
    std::pair<tier, CAmount> tierInfo = pmn->GetMasternodeTierInfo();

    if (pmn) {
        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("txhash", activeMasternode.vin.prevout.hash.ToString()));
        mnObj.push_back(Pair("outputidx", (uint64_t)activeMasternode.vin.prevout.n));
        mnObj.push_back(Pair("netaddr", activeMasternode.service.ToString()));
        mnObj.push_back(Pair("addr", CBitcoinAddress(pmn->pubKeyCollateralAddress.GetID()).ToString()));
        mnObj.push_back(Pair("status", activeMasternode.status));
        mnObj.push_back(Pair("tier", tierInfo.first));
        mnObj.push_back(Pair("message", activeMasternode.GetStatus()));
        return mnObj;
    }
    throw runtime_error("masternode not found in the list of available masternodes.\ncurrent status: "
                        + activeMasternode.GetStatus());
}

UniValue getmasternodewinners (const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "getmasternodewinners ( blocks \"filter\" )\n"
            "\nprint the masternode winners for the last n blocks\n"

            "\narguments:\n"
            "1. blocks      (numeric, optional) number of previous blocks to show (default: 10)\n"
            "2. filter      (string, optional) search filter matching MN address\n"

            "\nresult (single winner):\n"
            "[\n"
            "  {\n"
            "    \"nHeight\": n,           (numeric) block height\n"
            "    \"winner\": {\n"
            "      \"address\": \"xxxx\",    (string) liberty masternode address\n"
            "      \"nVotes\": n,          (numeric) number of votes for winner\n"
            "    }\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nresult (multiple winners):\n"
            "[\n"
            "  {\n"
            "    \"nHeight\": n,           (numeric) block height\n"
            "    \"winner\": [\n"
            "      {\n"
            "        \"address\": \"xxxx\",  (string) liberty masternode address\n"
            "        \"nVotes\": n,        (numeric) number of votes for winner\n"
            "      }\n"
            "      ,...\n"
            "    ]\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nexamples:\n" +
            HelpExampleCli("getmasternodewinners", "") + HelpExampleRpc("getmasternodewinners", ""));

    int nHeight;
    {
        LOCK(cs_main);
        CBlockIndex* pindex = chainActive.Tip();
        if(!pindex) return 0;
        nHeight = pindex->nHeight;
    }

    int nLast = 10;
    std::string strFilter = "";

    if (params.size() >= 1)
        nLast = atoi(params[0].get_str());

    if (params.size() == 2)
        strFilter = params[1].get_str();

    UniValue ret(UniValue::VARR);

    for (int i = nHeight - nLast; i < nHeight + 20; i++) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("nHeight", i));

        std::string strPayment = GetRequiredPaymentsString(i);
        if (strFilter != "" && strPayment.find(strFilter) == std::string::npos) continue;

        if (strPayment.find(',') != std::string::npos) {
            UniValue winner(UniValue::VARR);
            boost::char_separator<char> sep(",");
            boost::tokenizer< boost::char_separator<char> > tokens(strPayment, sep);
            BOOST_FOREACH (const string& t, tokens) {
                UniValue addr(UniValue::VOBJ);
                std::size_t pos = t.find(":");
                std::string strAddress = t.substr(0,pos);
                uint64_t nVotes = atoi(t.substr(pos+1));
                addr.push_back(Pair("address", strAddress));
                addr.push_back(Pair("nVotes", nVotes));
                winner.push_back(addr);
            }
            obj.push_back(Pair("winner", winner));
        } else if (strPayment.find("Unknown") == std::string::npos) {
            UniValue winner(UniValue::VOBJ);
            std::size_t pos = strPayment.find(":");
            std::string strAddress = strPayment.substr(0,pos);
            uint64_t nVotes = atoi(strPayment.substr(pos+1));
            winner.push_back(Pair("address", strAddress));
            winner.push_back(Pair("nVotes", nVotes));
            obj.push_back(Pair("winner", winner));
        } else {
            UniValue winner(UniValue::VOBJ);
            winner.push_back(Pair("address", strPayment));
            winner.push_back(Pair("nVotes", 0));
            obj.push_back(Pair("winner", winner));
        }

            ret.push_back(obj);
    }

    return ret;
}

UniValue getmasternodescores (const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getmasternodescores ( blocks )\n"
            "\nprint list of winning masternode by score\n"

            "\nArguments:\n"
            "1. blocks      (numeric, optional) show the last n blocks (default 10)\n"

            "\nResult:\n"
            "{\n"
            "  xxxx: \"xxxx\"   (numeric : string) block height : masternode hash\n"
            "  ,...\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("getmasternodescores", "") + HelpExampleRpc("getmasternodescores", ""));

    int nLast = 10;

    if (params.size() == 1) {
        try {
            nLast = std::stoi(params[0].get_str());
        } catch (const std::invalid_argument&) {
            throw runtime_error("exception on param 2");
        }
    }
    UniValue obj(UniValue::VOBJ);

    std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();
    for (int nHeight = chainActive.Tip()->nHeight - nLast; nHeight < chainActive.Tip()->nHeight + 20; nHeight++) {
        uint256 nHigh = 0;
        CMasternode* pBestMasternode = NULL;
        BOOST_FOREACH (CMasternode& mn, vMasternodes) {
            uint256 n = mn.CalculateScore(1, nHeight - 100);
            if (n > nHigh) {
                nHigh = n;
                pBestMasternode = &mn;
            }
        }
        if (pBestMasternode)
            obj.push_back(Pair(strprintf("%d", nHeight), pBestMasternode->vin.prevout.hash.ToString().c_str()));
    }

    return obj;
}

bool DecodeHexMnb(CMasternodeBroadcast& mnb, std::string strHexMnb) {

    if (!IsHex(strHexMnb))
        return false;

    vector<unsigned char> mnbData(ParseHex(strHexMnb));
    CDataStream ssData(mnbData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssData >> mnb;
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}
UniValue createmasternodebroadcast(const UniValue& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();
    if (fHelp || (strCommand != "alias" && strCommand != "all") || (strCommand == "alias" && params.size() < 2))
        throw runtime_error(
            "createmasternodebroadcast \"command\" ( \"alias\")\n"
            "\ncreates a masternode broadcast message for one or all masternodes configured in masternode.conf\n" +
            HelpRequiringPassphrase() + "\n"

            "\nArguments:\n"
            "1. \"command\"      (string, required) \"alias\" for single masternode, \"all\" for all masternodes\n"
            "2. \"alias\"        (string, required if command is \"alias\") alias of the masternode\n"

            "\nresult (all):\n"
            "{\n"
            "  \"overall\": \"xxx\",        (string) overall status message indicating number of successes.\n"
            "  \"detail\": [                (array) JSON array of broadcast objects.\n"
            "    {\n"
            "      \"alias\": \"xxx\",      (string) alias of the masternode.\n"
            "      \"success\": true|false, (boolean) auccess status.\n"
            "      \"hex\": \"xxx\"         (string, if success=true) hex encoded broadcast message.\n"
            "      \"error_message\": \"xxx\"   (string, if success=false) error message, if any.\n"
            "    }\n"
            "    ,...\n"
            "  ]\n"
            "}\n"

            "\nresult (alias):\n"
            "{\n"
            "  \"alias\": \"xxx\",      (string) alias of the masternode.\n"
            "  \"success\": true|false, (boolean) success status.\n"
            "  \"hex\": \"xxx\"         (string, if success=true) hex encoded broadcast message.\n"
            "  \"error_message\": \"xxx\"   (string, if success=false) error message, if any.\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("createmasternodebroadcast", "alias mymn1") + HelpExampleRpc("createmasternodebroadcast", "alias mymn1"));

    EnsureWalletIsUnlocked();

    if (strCommand == "alias")
    {
        // wait for reindex and/or import to finish
        if (fImporting || fReindex)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Wait for reindex and/or import to finish");

        std::string alias = params[1].get_str();
        bool found = false;

        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            if(mne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                CMasternodeBroadcast mnb;

                bool success = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb, true);

                statusObj.push_back(Pair("success", success));
                if(success) {
                    CDataStream ssMnb(SER_NETWORK, PROTOCOL_VERSION);
                    ssMnb << mnb;
                    statusObj.push_back(Pair("hex", HexStr(ssMnb.begin(), ssMnb.end())));
                } else {
                    statusObj.push_back(Pair("error_message", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("success", false));
            statusObj.push_back(Pair("error_message", "could not find alias in config; verify with list-conf."));
        }

        return statusObj;

    }

    if (strCommand == "all")
    {
        // wait for reindex and/or import to finish
        if (fImporting || fReindex)
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Wait for reindex and/or import to finish");

        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            std::string errorMessage;

            CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
            CMasternodeBroadcast mnb;

            bool success = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb, true);

            UniValue statusObj(UniValue::VOBJ);
            statusObj.push_back(Pair("alias", mne.getAlias()));
            statusObj.push_back(Pair("success", success));

            if(success) {
                successful++;
                CDataStream ssMnb(SER_NETWORK, PROTOCOL_VERSION);
                ssMnb << mnb;
                statusObj.push_back(Pair("hex", HexStr(ssMnb.begin(), ssMnb.end())));
            } else {
                failed++;
                statusObj.push_back(Pair("error_message", errorMessage));
            }

            resultsObj.push_back(statusObj);
        }

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("created broadcast messages:\nsuccess: %d\nfailed: %d\ntotal: %d\n", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }
    return NullUniValue;
}

UniValue decodemasternodebroadcast(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "decodemasternodebroadcast \"hexstring\"\n"
            "\ncommand to decode masternode broadcast messages\n"

            "\nargument:\n"
            "1. \"hexstring\"        (string) the hex encoded masternode broadcast message\n"

            "\nResult:\n"
            "{\n"
            "  \"vin\": \"xxxx\"                (string) the unspent output which is holding the masternode collateral\n"
            "  \"addr\": \"xxxx\"               (string) IP address of the masternode\n"
            "  \"pubkeycollateral\": \"xxxx\"   (string) collateral address's public key\n"
            "  \"pubkeymasternode\": \"xxxx\"   (string) masternode's public key\n"
            "  \"vchsig\": \"xxxx\"             (string) base64-encoded signature of this message (verifiable via pubkeycollateral)\n"
            "  \"sigtime\": \"nnn\"             (numeric) signature timestamp\n"
            "  \"protocolversion\": \"nnn\"     (numeric) masternode's protocol version\n"
            "  \"nlastdsq\": \"nnn\"            (numeric) the last time the masternode sent a DSQ message (for mixing) (DEPRECATED)\n"
            "  \"lastping\" : {                 (object) JSON object with information about the masternode's last ping\n"
            "      \"vin\": \"xxxx\"            (string) the unspent output of the masternode which is signing the message\n"
            "      \"blockhash\": \"xxxx\"      (string) current chaintip blockhash minus 12\n"
            "      \"sigtime\": \"nnn\"         (numeric) Signature time for this ping\n"
            "      \"vchsig\": \"xxxx\"         (string) base64-encoded signature of this ping (verifiable via pubkeymasternode)\n"
            "}\n"

            "\nexamples:\n" +
            HelpExampleCli("decodemasternodebroadcast", "hexstring") + HelpExampleRpc("decodemasternodebroadcast", "hexstring"));

    CMasternodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "masternode broadcast message decode failed");

    if(!mnb.VerifySignature())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "masternode broadcast signature verification failed");

    UniValue resultObj(UniValue::VOBJ);

    resultObj.push_back(Pair("vin", mnb.vin.prevout.ToString()));
    resultObj.push_back(Pair("addr", mnb.addr.ToString()));
    resultObj.push_back(Pair("pubkeycollateral", CBitcoinAddress(mnb.pubKeyCollateralAddress.GetID()).ToString()));
    resultObj.push_back(Pair("pubkeymasternode", CBitcoinAddress(mnb.pubKeyMasternode.GetID()).ToString()));
    resultObj.push_back(Pair("vchsig", EncodeBase64(&mnb.sig[0], mnb.sig.size())));
    resultObj.push_back(Pair("sigtime", mnb.sigTime));
    resultObj.push_back(Pair("protocolversion", mnb.protocolVersion));
    resultObj.push_back(Pair("nlastdsq", mnb.nLastDsq));

    UniValue lastPingObj(UniValue::VOBJ);
    lastPingObj.push_back(Pair("vin", mnb.lastPing.vin.prevout.ToString()));
    lastPingObj.push_back(Pair("blockhash", mnb.lastPing.blockHash.ToString()));
    lastPingObj.push_back(Pair("sigtime", mnb.lastPing.sigTime));
    lastPingObj.push_back(Pair("vchsig", EncodeBase64(&mnb.lastPing.vchSig[0], mnb.lastPing.vchSig.size())));

    resultObj.push_back(Pair("lastping", lastPingObj));

    return resultObj;
}

UniValue relaymasternodebroadcast(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "relaymasternodebroadcast \"hexstring\"\n"
            "\ncommand to relay masternode broadcast messages\n"

            "\narguments:\n"
            "1. \"hexstring\"        (string) The hex encoded masternode broadcast message\n"

            "\nexamples:\n" +
            HelpExampleCli("relaymasternodebroadcast", "hexstring") + HelpExampleRpc("relaymasternodebroadcast", "hexstring"));


    CMasternodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "masternode broadcast message decode failed");

    if(!mnb.VerifySignature())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "masternode broadcast signature verification failed");

    mnodeman.UpdateMasternodeList(mnb);
    mnb.Relay();

    return strprintf("masternode broadcast sent (service %s, vin %s)", mnb.addr.ToString(), mnb.vin.ToString());
}
