// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX Developers 
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include "pow.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

static bool regenerate = false;

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (60185, uint256("0x056487b30fdf0a757477d2d097029ce73f92b3e876c969dc37fe5ea5488eeb6e")); 
static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1545670556, // * UNIX timestamp of last checkpoint block
    133104,    // * total number of transactions between genesis and last checkpoint
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
        nRpcPort = 10416;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // Liberty starting difficulty is 1 / 2^12
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 8100; // 75%
        nRejectBlockOutdatedMajority = 10260; // 95%
        nToCheckBlockUpgradeMajority = 10800; // Approximate expected amount of blocks in 7 days (1440*7.5)
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; 
        nTargetSpacing = 1 * 60; 
        nMaturity = 100;
        nMaxMoneyOut = 9999999999 * COIN;
        nLastPoWBlock = 200;
        nCheckLockTimeVerify_StartTime = 1547078401; // Thursday, January 10, 2019 12:00:01 AM
        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
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
        genesis.nVersion = 4;
        genesis.nAccumulatorCheckpoint = 0;
        genesis.nTime = 1542060000;
        genesis.nBits = 504365040;
        genesis.nNonce = 115752;

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
            assert(hashGenesisBlock == uint256("0x000007c2691250ffc124372e912f9dea4272240040363ac0d7a394fade8a4798"));
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
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "047d1cf865235053c5b88418dbe16b72991535d234a00a7bb08cfbadbf6dd9ad55283ea5d56ef75adc1b9945b8a70f576a758e23c6c69abf447abe03a35f9c5933";
        strObfuscationPoolDummyAddress = "XaCWffkq3XKSgqGD8TfqFKrzoWKCDknCkV";
        
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
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a xlibz to be stakable
        nStakeMinAge = 60 * 60; //The number of seconds that a utxo must be old before it can qualify for staking
        nStakeMinConfirmations = 60; //The number of confirmations that a utxo must have before it can qualify for staking
        nStakeMinAmount = 100 * COIN; //The minimum number of coins that can be staked.
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
        nRpcPort = 20416;
        nEnforceBlockUpgradeMajority = 4320; // 75%
        nRejectBlockOutdatedMajority = 5472; // 95%
        nToCheckBlockUpgradeMajority = 5760; // 4 days
        nMinerThreads = 0;
        nTargetTimespan = 1 * 60; // Liberty: 1 day
        nTargetSpacing = 1 * 60;  // Liberty: 1 minute
        nLastPoWBlock = 200;
        nMaturity = 15;
        nMaxMoneyOut = 43199500 * COIN;
        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1540073799;
        genesis.nNonce = 703542;

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
            assert(hashGenesisBlock == uint256("0x00000f4c080d58c3ce9ec79d7ecd8504b07675d6233a8373298f20ce865ccef0"));
            assert(genesis.hashMerkleRoot == uint256("0xbff44217a82b3a7975225053503c553bc63b0f594f51a15ee2d9a5007c281b01"));
        }

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

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 81); // Testnet liberty addresses start with 'Z'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 11);  // Testnet liberty script addresses start with '5'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet liberty BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet liberty BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet liberty BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

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
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 24 * 60 * 60; // Liberty: 1 day
        nTargetSpacing = 1 * 60;        // Liberty: 1 minutes
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = 1454124731;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 12345;

        hashGenesisBlock = genesis.GetHash();
        nP2pPort = 51476;
       // assert(hashGenesisBlock == uint256("0x4f023a2120d9127b21bbad01724fdb79b519f593f2a85b60d3d79160ec5f29df"));

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
        nP2pPort = 51478;
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
