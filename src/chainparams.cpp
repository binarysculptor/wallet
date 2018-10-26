// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

static bool regenerate = false;
/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (0, uint256("0x001"));
static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1525106065, // * UNIX timestamp of last checkpoint block
    2498834,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1740710,
    0,
    250};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

libzerocoin::ZerocoinParams* CChainParams::Zerocoin_Params() const
{
    assert(this);
    static CBigNum bnDecModulus = 0;
    if (!bnDecModulus)
        bnDecModulus.SetDec(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsDec = libzerocoin::ZerocoinParams(bnDecModulus);

    return &ZCParamsDec;
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xac;
        pchMessageStart[1] = 0xae;
        pchMessageStart[2] = 0xaf;
        pchMessageStart[3] = 0x8f;
        vAlertPubKey = ParseHex("04e0b61287c79205a4d894963238cc5d0227fe95eead9573ccd9b20debb71022b0f18a6406dd798bdbe3cbd9c801d9b5f7df5c045f709d816d406dc828172e3bac");
        nP2pPort = 10417;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // PIVX starting difficulty is 1 / 2^12
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 8100; // 75%
        nRejectBlockOutdatedMajority = 10260; // 95%
        nToCheckBlockUpgradeMajority = 10800; // Approximate expected amount of blocks in 7 days (1440*7.5)
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; // PIVX: 1 day
        nTargetSpacing = 1 * 60;  // PIVX: 1 minute
        nMaturity = 100;
        nMaxMoneyOut = 999999999 * COIN; //100B -1 XLB max supply

        /** Height or Time Based Activations **/
        nLastPoWBlock = 225;
        nModifierUpdateBlock = 1;
        nZerocoinActivationBlock = 245;
        nZercoinActivationTime = 1540857600;

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "Bitcoin's White Paper Gave Us Liberty – Let's Not Give It Back | Coindesk | 10-20-2018";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 250 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = 1540072799;
        genesis.nBits = 504365040;
        genesis.nNonce = 1366751;

        //For all those who fork Liberty coin:
        //We bless you with the gift of a genesis block creator (courtesy of Helium coin); go make something great!
        //Remember, please, to keep the copyright notices in place in all of the source files.
        hashGenesisBlock = genesis.GetHash();
        if (regenerate) {
            hashGenesisBlock = uint256S("");
            genesis.nNonce = 0;
            if (true && (genesis.GetHash() != hashGenesisBlock)) {
                while (genesis.GetHash() > bnProofOfWorkLimit)
                {
                    ++genesis.nNonce;
                    if (genesis.nNonce == 0)
                    {
                        ++genesis.nTime;
                    }
                }
                std::cout << "// Mainnet ---";
                std::cout << " nonce: " << genesis.nNonce;
                std::cout << " time: " << genesis.nTime;
                std::cout << " hash: 0x" << genesis.GetHash().ToString().c_str();
                std::cout << " merklehash: 0x"  << genesis.hashMerkleRoot.ToString().c_str() <<  "\n";
            }
        } else {
            assert(hashGenesisBlock == uint256("0x00000c14d49f1caea9efa0dfdff9920de60e1ba586b3ff1cf07e905f8c012321"));
            assert(genesis.hashMerkleRoot == uint256("0xbff44217a82b3a7975225053503c553bc63b0f594f51a15ee2d9a5007c281b01"));
        }

        vSeeds.push_back(CDNSSeedData("dns-1", "seedns1.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("dns-2", "seedns2.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("dns-3", "seedns3.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("dns-4", "seedns4.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("dns-5", "seedns5.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-1", "node-01.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-2", "node-02.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-3", "node-03.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-4", "node-04.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-5", "node-05.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-6", "node-06.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-7", "node-07.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-8", "node-08.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-9", "node-09.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-10", "node-10.liberty-coin.com"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 75); //'X'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 18); // '8'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 211); // '8'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0x2D)(0x25)(0x33).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x31)(0x2B).convert_to_container<std::vector<unsigned char> >();
        // 	BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "047d1cf865235053c5b88418dbe16b72991535d234a00a7bb08cfbadbf6dd9ad55283ea5d56ef75adc1b9945b8a70f576a758e23c6c69abf447abe03a35f9c5933";
        strObfuscationPoolDummyAddress = "D87q2gC9j6nNrnzCsg4aY6bHMLsT9nUhEw";

        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        nZerocoinHeaderVersion = 4; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zpiv to be stakable
        nStakeMinAge = 60 * 60 * 3; //The number of seconds that a utxo must be old before it can qualify for staking
        nBudget_Fee_Confirmations = 6; // Number of confirmations for the finalization fee
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x8f;
        pchMessageStart[1] = 0xaf;
        pchMessageStart[2] = 0xae;
        pchMessageStart[3] = 0xac;
        vAlertPubKey = ParseHex("04746091c820e6b051fd350f856fc75de673d69993487489dc091c9139b0de345479238306cc98ccf0b1a30c338b99636d67881aece3a906f03ae5eb4d51f6462e");
        nP2pPort = 20417;
        nEnforceBlockUpgradeMajority = 4320; // 75%
        nRejectBlockOutdatedMajority = 5472; // 95%
        nToCheckBlockUpgradeMajority = 5760; // 4 days
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; // PIVX: 1 day
        nTargetSpacing = 1 * 60;  // PIVX: 1 minute
        nLastPoWBlock = 200;
        nZerocoinActivationBlock = 220;
        nZercoinActivationTime = 1540857600;
        nMaturity = 15;
        nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 625000000 * COIN; //625M XLB max supply

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1540073799;
        genesis.nNonce = 1613928;

        hashGenesisBlock = genesis.GetHash();
        if (regenerate) {
            hashGenesisBlock = uint256S("");
            genesis.nNonce = 0;
            if (true && (genesis.GetHash() != hashGenesisBlock)) {
                while (genesis.GetHash() > bnProofOfWorkLimit)
                {
                    ++genesis.nNonce;
                    if (genesis.nNonce == 0)
                    {
                        ++genesis.nTime;
                    }
                }
                std::cout << "// Testnet ---";
                std::cout << " nonce: " << genesis.nNonce;
                std::cout << " time: " << genesis.nTime;
                std::cout << " hash: 0x" << genesis.GetHash().ToString().c_str();
                std::cout << " merklehash: 0x"  << genesis.hashMerkleRoot.ToString().c_str() <<  "\n";

            }
        } else {
            assert(hashGenesisBlock == uint256("0x00000583310b89a91d9348f4d78d7af1ae6035b5f7f038bd95a59ec8d6605539"));
            assert(genesis.hashMerkleRoot == uint256("0xbff44217a82b3a7975225053503c553bc63b0f594f51a15ee2d9a5007c281b01"));
        }

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("node-1", "node-01.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-2", "node-02.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-3", "node-03.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-4", "node-04.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-5", "node-05.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-6", "node-06.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-7", "node-07.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-8", "node-08.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-9", "node-09.liberty-coin.com"));
        vSeeds.push_back(CDNSSeedData("node-10", "node-10.liberty-coin.com"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 81); // Testnet pivx addresses start with 'Z'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 11);  // Testnet pivx script addresses start with '5'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet pivx BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet pivx BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet pivx BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        strSporkKey = "04e9e2881b595229477d0e91a35b5013e9b425c096ee7f2d1fb0be8d25afe7fafe195e1c281cb37d8c56b7c7cbcac5ca8b0f6c6bc276ccb024e10563ddc863e46a";
        strObfuscationPoolDummyAddress = "y57cqfGRkekRyDRNeJiLtYVEbvhXrNbmox";
        nBudget_Fee_Confirmations = 3; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        strNetworkID = "regtest";
        pchMessageStart[0] = 0x8f;
        pchMessageStart[1] = 0xaf;
        pchMessageStart[2] = 0xae;
        pchMessageStart[3] = 0xac;
        nP2pPort = 30417;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 24 * 60 * 60; // PIVX: 1 day
        nTargetSpacing = 1 * 60;        // PIVX: 1 minutes
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = 1454124731;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 12345;

        hashGenesisBlock = genesis.GetHash();

        //assert(hashGenesisBlock == uint256("0x4f023a2120d9127b21bbad01724fdb79b519f593f2a85b60d3d79160ec5f29df"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nP2pPort = 40417;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
