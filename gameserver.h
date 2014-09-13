#ifndef GAMESERVER_H
#define GAMESERVER_H

#include "singleton.h"
#include "wordfilter.h"
#include "lvsys.h"
#include <stdlib.h>
#include "gameserverproto.h"


double Distance(GMDT_POINT roPoint1, GMDT_POINT roPoint2);

class CGameServer
{
    CGameServer(void);
    virtual ~CGameServer();

    DECLARE_SINGLETON(CGameServer)
public:
    
    bool Init();
    void Uninit();
    void MainLoop();
    void SavingData();

    INT32 GetGroupID() { return m_nGroupID; }
    INT32 GetServerID() { return m_nGSID; }
    string GetPingTai() { return m_strPingTai; }
    string GetIP() { return m_strIP; }
    UINT16 GetPort() { return m_wPort; }
	void SetTerminate(bool bTerminate) { m_bTerminate = bTerminate; }
    UINT8 GetIsReport() { return m_byIsReport;}

    CWordFilterUTF8* GetWordFilter() { return &m_oWordFilter; }

protected:
    bool _ReadConfig();
    bool _LoadTableValue();

    void _CreateAllInstance();
    void _DestroyAllInstance();

    bool _InitLogger();
    void _UnInitLogger();

    bool _InitNet();
    void _UnInitNet();

    bool _InitDB();
    void _UnInitDB();

    bool _InitPlayerMgr();
    void _UnInitPlayerMgr();

    bool _InitBuddyMgr();
    void _UnInitBuddyMgr();

    bool _InitSceneMgr();
    void _UnInitSceneMgr();

    bool _InitNpcMgr();
    void _UnInitNpcMgr();

    bool _InitMonsterMgr();
    void _UnInitMonsterMgr();

    bool _InitSkillMgr();
    void _UnInitSkillMgr();

    bool _InitItemMgr();
    bool _InitRaidMgr();
    void _UnInitItemMgr();

    bool _InitGMCmdMgr();
    void _UnInitGMCmdMgr();

    bool _InitPropAddonMgr();
    void _UnInitPropAddonMgr();
    
    bool _InitCGuildMgr();
    void _UnInitCGuildMgr();

    bool _InitGuildBattleMgr();

    bool _InitBoardMgr();
    void _UnInitBoardMgr();

    void _TestKeyWord();
    void _TestTable();
	bool _CheckTables();

    bool _InitMapHistoryMgr();
    void _UnInitMapHistoryMgr();

    bool _InitShopMgr();
    void _UnInitShopMgr();

    bool _InitScheduleMgr();
    void _UnInitScheduleMgr();

    bool _InitActivityMgr();
    void _UnInitActivityMgr();

    bool _InitArenaMgr();
    void _UnInitArenaMgr();

    bool _InitShmmgr();
    void _UnInitShmmgr();

	bool _InitYunWeiMgr();
	void _UnInitYunWeiMgr();

    bool _InitChatMgr();
    void _UnInitChatMgr();

    bool _InitMailMgr();
    void _UnInitMailMgr();

    bool _InitUnitSceneMgr();
    void _UnInitUnitSceneMgr();

	bool _InitTeamMgr();
	void _UnInitTeamMgr();

    bool _InitDoodadMgr();
    void _UnInitDoodadMgr();

    bool _InitTradeMgr();
    void _UnInitTradeMgr();

    bool _InitTaskMgr();
    void _UnInitTaskMgr();

public:
    bool    m_bTerminate;
    std::string m_strPingTai;
    INT32   m_nGroupID;
    INT32   m_nGSID;
    string  m_strIP;
    UINT16  m_wPort;
    INT32   m_nMaxPlayerNum;
    UINT8   m_byIsReport;
    INT32   m_nCurOfflineTaskID;
    CWordFilterUTF8 m_oWordFilter;

    UINT32  m_dwMaxFrame;
    UINT32  m_dwLastMaxFrameByHour;
    UINT32  m_dwCurFrame;
    UINT32  m_dwFrameCountOver1;
    UINT32  m_dwFrameCountOver2;

    SLVTimeVal m_stLastUpdateByHour;
    SLVTimeVal m_stLastUpdateByFrame;

    bool    m_bLastCrash;
};

#endif
