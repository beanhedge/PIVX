// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "primitives/zerocoin.h"
#include "main.h"
#include "stakeinput.h"
#include "wallet.h"

CBlockIndex* CZPivStake::GetIndexFrom()
{
   // if (pindexFrom)
     //   return pindexFrom;

    //todo - match up acc checksum from spend object to a block height
//    int nTxHeight = mint.GetHeight();
//    int nAccumulatedHeight = (10 - nTxHeight % 10) + 10 + nTxHeight;

//    if (chainActive.Height() < nAccumulatedHeight)
//        return nullptr;
//
//    pindexFrom = chainActive[nAccumulatedHeight];
    //return pindexFrom;
    return nullptr;
}

CAmount CZPivStake::GetValue()
{
    return spend->getDenomination() * COIN;
}

//Use the first accumulator checkpoint that occurs 60 minutes after the block being staked from
bool CZPivStake::GetModifier(uint64_t& nStakeModifier)
{
    CBlockIndex* pindex = GetIndexFrom();
    if (!pindex)
        return false;

    int64_t nTimeBlockFrom = pindex->GetBlockTime();
    while (true) {
        if (pindex->GetBlockTime() - nTimeBlockFrom > 60*60) {
            nStakeModifier = pindex->nAccumulatorCheckpoint.Get64();
            return true;
        }

        if (pindex->nHeight + 1 <= chainActive.Height())
            pindex = chainActive.Next(pindex);
        else
            return false;
    }
}

CDataStream CZPivStake::GetUniqueness()
{
    CDataStream ss(SER_GETHASH, 0);
    ss << spend->getCoinSerialNumber();
    return ss;
}

bool CZPivStake::CreateTxIn(CTxIn& txIn)
{
    return true;
}

bool CZPivStake::GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey)
{
    return true;
}

bool CZPivStake::GetTxFrom(CTransaction& tx)
{
    return false;
}


//!PIV Stake
bool CPivStake::SetInput(std::pair<CTransaction*, unsigned int> input)
{
    this->pcoin = input;
    return true;
}

bool CPivStake::GetTxFrom(CTransaction& tx)
{
    tx = *pcoin.first;
    return true;
}

bool CPivStake::CreateTxIn(CTxIn& txIn)
{
    txIn = CTxIn(pcoin.first->GetHash(), pcoin.second);
    return true;
}

CAmount CPivStake::GetValue()
{
    return pcoin.first->vout[pcoin.second].nValue;
}

bool CPivStake::GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey)
{
    vector<valtype> vSolutions;
    txnouttype whichType;
    CScript scriptPubKeyOut;
    CScript scriptPubKeyKernel = pcoin.first->vout[pcoin.second].scriptPubKey;
    if (!Solver(scriptPubKeyKernel, whichType, vSolutions)) {
        LogPrintf("CreateCoinStake : failed to parse kernel\n");
        return false;
    }

    if (whichType != TX_PUBKEY && whichType != TX_PUBKEYHASH)
        return false; // only support pay to public key and pay to address

    if (whichType == TX_PUBKEYHASH) // pay to address type
    {
        //convert to pay to public key type
        CKey key;
        if (!keystore.GetKey(uint160(vSolutions[0]), key))
            return false;

        scriptPubKeyOut << key.GetPubKey() << OP_CHECKSIG;
    } else
        scriptPubKeyOut = scriptPubKeyKernel;

    return true;
}

bool CPivStake::GetModifier(uint64_t& nStakeModifier)
{
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;
    GetIndexFrom();
    if (!pindexFrom)
        return false;

    if (!GetKernelStakeModifier(pindexFrom->GetBlockHash(), nStakeModifier, nStakeModifierHeight, nStakeModifierTime, false)) {
        LogPrintf("CheckStakeKernelHash(): failed to get kernel stake modifier \n");
        return false;
    }

    return true;
}

CDataStream CPivStake::GetUniqueness()
{
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    return ss;
}

CBlockIndex* CPivStake::GetIndexFrom()
{
    return nullptr;
}