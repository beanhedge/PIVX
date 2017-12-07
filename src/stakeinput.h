// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_STAKEINPUT_H
#define PIVX_STAKEINPUT_H

class CKeyStore;
class CWalletTx;

class CStakeInput
{
protected:
    CBlockIndex* pindexFrom;

public:
    virtual CBlockIndex* GetIndexFrom() = 0;
    virtual bool CreateTxIn(CTxIn& txIn) = 0;
    virtual CAmount GetValue() = 0;
    virtual bool GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey) = 0;
    virtual bool GetModifier(uint64_t& nStakeModifier) = 0;
    virtual CDataStream GetUniqueness() = 0;
};

class CZPivStake : public CStakeInput
{
private:
    libzerocoin::CoinSpend spend;

public:
    void SetSpend(libzerocoin::CoinSpend spend) { this->spend = spend; }
    CBlockIndex* GetIndexFrom() override;
    CAmount GetValue() override;
    bool GetModifier(uint64_t& nStakeModifier) override;
    CDataStream GetUniqueness() override;
};

class CPivStake : public CStakeInput
{
private:
    std::pair<const CWalletTx*, unsigned int> pcoin;
public:
    void SetInput(const CTransaction& tx, unsigned int pos);
    bool CreateTxIn(CTxIn& txIn) override;
    CAmount GetValue() override;
    bool GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey) override;
    bool GetModifier(uint64_t& nStakeModifier) override;
};


#endif //PIVX_STAKEINPUT_H
