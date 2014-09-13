#include "gamepch.h"
#include "gameserver.h"
#include "markupstl.h"
#include "netctrl.h"
#include "dbctrl.h"
#include "playermgr.h"
#include "buddymgr.h"
#include "tablemgr.h"
#include "tblhelper.h"
#include "scenemgr.h"
#include "npcmgr.h"
#include "monstermgr.h"
#include "skillmgr.h"
#include "itemmgr.h"
#include "shopmgr.h"
#include "gmcmd.h"
#include "prop.h"
#include "guildmgr.h"
#include "board.h"
#include "schedule.h"
#include "activitymgr.h"
#include "chatmgr.h"
#include "mapvideo.h"
#include "arenamgr.h"
#include "slavehandler.h"
#include "tblparam.h"
#include "shmmgr.h"
#include "stackrecorder.h"
#include "tblmarket.h"
#include "yunweimgr.h"
#include "mailmgr.h"
#include "unitscenemgr.h"
#include <math.h>
#include "team.h"
#include "doodad.h"
#include "raidmgr.h"
#include "trademgr.h"
#include "guildbattle.h"
#include "taskbag.h"
#include "storytasklist.h"

#define MAX_GROUP_ID    1024
#define MAX_SERVER_ID   1000000
#define MAX_PLAYER_NUM  1000

CLVRandom   g_oRandom;
SLVTimeVal  g_oGlobalDBGTimeVal;

double Distance(GMDT_POINT roPoint1, GMDT_POINT roPoint2)
{
    return sqrt((roPoint1.m_nX - roPoint2.m_nX)*(roPoint1.m_nX - roPoint2.m_nX) + 
        (roPoint1.m_nY - roPoint2.m_nY)*(roPoint1.m_nY - roPoint2.m_nY));
}

IMPLEMENT_SINGLETON(CGameServer);

CGameServer::CGameServer()
{
    m_bTerminate    = false;
    m_nGroupID      = 0;
    m_nGSID     = 0;
    m_strPingTai    = "";
}

CGameServer::~CGameServer()
{

}

bool CGameServer::Init()
{
    m_bTerminate = false;
    m_bLastCrash = false;

    SLVTimeVal stTime;
    LVSYS_GetTimeEx(&stTime);
    g_oRandom.SetSeed(stTime.m_dwSec + stTime.m_dwMsec);

    m_dwMaxFrame = 0;
    m_dwLastMaxFrameByHour = 0;
    m_dwCurFrame = 0;
    m_dwFrameCountOver1 = 0;
    m_dwFrameCountOver2 = 0;

    m_nCurOfflineTaskID = 0;

    if(false == _InitLogger())
        return false;

    WRN("Begin to start gameserver ........");

    if(false == _ReadConfig())
        return false;

    if(false == LoadAllTables())
        return false;

	if(false == _CheckTables() )
	{
		return false;
	}

    CShmMgr::CreateInstance();
    if(false == _InitShmmgr())
    {
        return false;
    }

    if(false == LV_GetStackRecorder(m_nGSID, m_nGSID))
    {
        CRI("CGameServer::Init LV_GetStackRecorder Fail");
        return false;
    }

    _CreateAllInstance();

    if(false == _InitPropAddonMgr())
        return false;

    if(false == _InitGMCmdMgr())
        return false;

    if(false == _InitItemMgr())
        return false;

    if(false == _InitRaidMgr())
        return false;
    if(false == _InitNet())
        return false;

    if(false == _InitScheduleMgr())
    {
        return false;
    }

    if(false == _InitMapHistoryMgr())
        return false;

    if(false == _InitSkillMgr())
        return false;

    if(false == _InitMonsterMgr())
        return false;

    if(false == _InitSceneMgr())
        return false;

    if(false == _InitTradeMgr())
    {
        return false;
    }

    if(false == _InitTaskMgr())
        return false;

    if(false == _InitDB())
        return false;

    if(false == _InitMailMgr())
        return false;

    if(false == _InitNpcMgr())
        return false;

    if(false == _InitPlayerMgr())
        return false;

    if(false == _InitBuddyMgr())
    {
        return false;
    }
    
    if(false == _InitCGuildMgr())
        return false;

    if(false == _InitBoardMgr())
        return false;

    if(false == _InitShopMgr())
        return false;

    if(false == _InitArenaMgr())
        return false;

    if(false == _InitActivityMgr())
    {
        return false;
    }

    if (false == _InitGuildBattleMgr()){
        return false;
    }

    if(false == _InitChatMgr())
    {
        return false;
    }

	if(false == _InitYunWeiMgr() )
	{
		return false;
	}

    if(false == _InitUnitSceneMgr())
    {
        return false;
    }

	if( false == _InitTeamMgr() )
	{
		return false;
	}

    if(false == _InitDoodadMgr())
    {
        return false;
    }

    CStoryTaskList::Instance()->Init();

    

    _LoadTableValue();
    LVSYS_GetTimeEx(&stTime);
    m_stLastUpdateByFrame = stTime;
    m_stLastUpdateByHour = stTime;

	CDBCtrl::Instance()->LoadTeamMembers();
	while( CDBCtrl::Instance()->m_dwCurDBCmdCount != 0  )
	{
		CDBCtrl::Instance()->UPdate();
		LVSYS_Sleep(1);
	}

#ifdef AUTH	
	CNetCtrl::Instance()->ConnectToAuth();
#endif

    WRN("Start gameserver OK!");
    return true;
}

void CGameServer::Uninit()
{
    WRN("Begin to shutdown gameserver......");
    
    
    _UnInitChatMgr();
    _UnInitActivityMgr();
    _UnInitArenaMgr();
    _UnInitShopMgr();
    _UnInitBoardMgr();
    _UnInitCGuildMgr();
    _UnInitBuddyMgr();
    _UnInitPlayerMgr();
    _UnInitNpcMgr();
    _UnInitMailMgr();
    _UnInitDB();
    _UnInitTaskMgr();
    _UnInitTradeMgr();
    _UnInitSceneMgr();
    _UnInitMonsterMgr();
    _UnInitSkillMgr();
    _UnInitMapHistoryMgr();
    _UnInitScheduleMgr();
    _UnInitNet();
    _UnInitItemMgr();
    _UnInitGMCmdMgr();
    _UnInitPropAddonMgr();
    _UnInitUnitSceneMgr();
    _UnInitTeamMgr();
    _UnInitDoodadMgr();

    _DestroyAllInstance();

    DBG("Uninit _DestroyAllInstance OK!");
    
    _UnInitLogger();
    
    DBG("Uninit _UnInitLogger OK!");

    CStackRecorder::Instance()->UnInit();
    CStackRecorder::DestoryInstance();

    DBG("Uninit CStackRecorder OK!");

    CShmMgr::Instance()->Uninit();
    CShmMgr::DestoryInstance();

    DBG("Shutdown gameserver OK! ");
}

bool CGameServer::_ReadConfig()
{
    CMarkupSTL oXml;
    if(false == oXml.Load(GS_CONF))
    {
        CRI("CGameServer::_ReadConfig, Load Config File %s failed: %s", GS_CONF, oXml.GetError().c_str());
        return false;
    }

    if(false == oXml.FindElem("config"))
    {
        CRI("CGameServer::_ReadConfig, Can not find element config");
        return false;
    }
    if(false == oXml.IntoElem())
    {
        CRI("CGameServer::_ReadConfig, Fail to into element config");
        return false;
    }
    if(false == oXml.FindElem("server"))
    {
        CRI("CGameServer::_ReadConfig, Can not Find element server");
        return false;
    }

    //string strPingTai = oXml.GetAttrib("pingtai");
    //if(strPingTai.empty())
    //{
    //    CRI("CGameServer::_ReadConfig, can not find pingtai");
    //    return false;
    //}
    m_strPingTai = "QQ";

    string strGroupID = oXml.GetAttrib("groupid");
    if(strGroupID.empty())
    {
        CRI("CGameServer::_ReadConfig, can not find groupid");
        return false;
    }
    m_nGroupID = atoi(strGroupID.c_str());
    if(m_nGroupID <= 0 || m_nGroupID >= MAX_GROUP_ID)
    {
        CRI("CGameServer::_ReadConfig, groupid %d error", m_nGroupID);
        return false;
    }

    string strServerID = oXml.GetAttrib("gsid");
    if(strServerID.empty())
    {
        CRI("CGameServer::_ReadConfig, can not find gsid");
        return false;
    }
    m_nGSID = atoi(strServerID.c_str());
    if(m_nGSID <= 0 || m_nGSID >= MAX_SERVER_ID)
    {
        CRI("CGameServer::_ReadConfig, serverid %d error", m_nGSID);
        return false;
    }

    if(false == m_oWordFilter.Init("wordlist.txt"))
    {
        CRI("CGameServer::_ReadConfig, load wordlist failed");
        return false;
    }

    oXml.ResetPos();
    if(false == oXml.FindElem("config"))
    {
        CRI("CGameServer::_ReadConfig, Can not find element config");
        return false;
    }
    if(false == oXml.IntoElem())
    {
        CRI("CGameServer::_ReadConfig, Fail to into element config");
        return false;
    }
    if(false == oXml.FindElem("listen"))
    {
        CRI("CGameServer::_ReadConfig, Can not Find element listen");
        return false;
    }
    m_strIP = oXml.GetAttrib("ip");
    if(m_strIP.empty())
    {
        CRI("CGameServer::_ReadConfig, can not get ip attr");
        return false;
    }
    string strPort = oXml.GetAttrib("port");
    if(strPort.empty())
    {
        CRI("CGameServer::_ReadConfig, can not get port");
        return false;
    }
    m_wPort = atoi(strPort.c_str());


    oXml.ResetPos();
    if(false == oXml.FindElem("config"))
    {
        CRI("CGameServer::_ReadConfig, Can not find element config");
        return false;
    }

    if(false == oXml.IntoElem())
    {
        CRI("CGameServer::_ReadConfig, Fail to into element config");
        return false;
    }

    if(false == oXml.FindElem("game"))
    {
        m_nMaxPlayerNum = MAX_PLAYER_NUM;
        DBG("CGameServer::_ReadConfig Max Player Num MAX_PLAYER_NUM %d", m_nMaxPlayerNum);
        return true;
    }

    string strPlayerNum = oXml.GetAttrib("playernum");
    if(strPlayerNum.empty())
    {
        CRI("CGameServer::_ReadConfig, can not get playernum");
        return false;
    }

    m_nMaxPlayerNum = atoi(strPlayerNum.c_str());
    DBG("CGameServer::_ReadConfig Max Player Num %d", m_nMaxPlayerNum);

    string strReport = oXml.GetAttrib("report");
    if(strReport.empty())
    {
        m_byIsReport = 0;
    }
    else
    {
        m_byIsReport = (strReport == "y" || strReport == "Y") ? 1 : 0;
    }
    DBG("CGameServer::_ReadConfig Is Report %u", (UINT8)m_byIsReport);

    //_TestKeyWord();

    return true;
}

void CGameServer::_CreateAllInstance()
{
    CNetCtrl::CreateInstance();
    CDBCtrl::CreateInstance();
    CBuddyMgr::CreateInstance();
    CPlayerMgr::CreateInstance();
    CSceneMgr::CreateInstance();
    CNpcMgr::CreateInstance();
    CMonsterMgr::CreateInstance();
    CSkillMgr::CreateInstance();
    CItemMgr::CreateInstance();
    CGMCmdMgr::CreateInstance();
    CActivityMgr::CreateInstance();
    CChatMgr::CreateInstance();
    CScheduleMgr::CreateInstance();
    CPropAddonMgr::CreateInstance();
    CMapVideoMgr::CreateInstance();
    CShopMgr::CreateInstance();
    CGuildMgr::CreateInstance();
    CBoardMgr::CreateInstance();
    CArenaMgr::CreateInstance();
	CSlaveHandler::CreateInstance();
    CShmMgr::CreateInstance();
	CYunWeiMgr::CreateInstance();
    CMailMgr::CreateInstance();
    CUnitSceneMgr::CreateInstance();
	CTeamMgr::CreateInstance();
    CDoodadMgr::CreateInstance();
    CRaidMgr::CreateInstance();
    CTradeMgr::CreateInstance();
    CTaskMgr::CreateInstance();
    CGuildBattleMgr::CreateInstance();
    CStoryTaskList::CreateInstance();
}

void CGameServer::_DestroyAllInstance()
{
    CTradeMgr::DestoryInstance();
    CMailMgr::DestoryInstance();
    CSlaveHandler::DestoryInstance();
    CNetCtrl::DestoryInstance();
    CDBCtrl::DestoryInstance();
    CPlayerMgr::DestoryInstance();
    CBuddyMgr::DestoryInstance();
    CSceneMgr::DestoryInstance();
    CNpcMgr::DestoryInstance();
    CMonsterMgr::DestoryInstance();
    CSkillMgr::DestoryInstance();
    CItemMgr::DestoryInstance();
    CGMCmdMgr::DestoryInstance();
    CChatMgr::DestoryInstance();
    CActivityMgr::DestoryInstance();
    CScheduleMgr::DestoryInstance();
    CPropAddonMgr::DestoryInstance();
    CMapVideoMgr::DestoryInstance();
    CShopMgr::DestoryInstance();
    CGuildMgr::DestoryInstance();
    CBoardMgr::DestoryInstance();
    CArenaMgr::DestoryInstance();
    CUnitSceneMgr::DestoryInstance();
	CTeamMgr::DestoryInstance();
    CDoodadMgr::DestoryInstance();
}

bool CGameServer::_InitLogger()
{
    if(false == CAppLogger::Init(GS_CONF, "gsapp"))
        return false;

    INF("CGameServer::_InitLogger success\n");

    return true;
}

void CGameServer::_UnInitLogger()
{
    CAppLogger::UnInit();
}

bool CGameServer::_InitNet()
{
    if(false == CNetCtrl::Instance()->Init())
        return false;

    return true;
}

void CGameServer::_UnInitNet()
{
    if(CNetCtrl::Instance() == NULL)
        return;

    CNetCtrl::Instance()->UnInit();
}

bool CGameServer::_InitDB()
{
    if(false == CDBCtrl::Instance()->Init())
        return false;

    return true;
}

void CGameServer::_UnInitDB()
{
    if(CDBCtrl::Instance() == NULL)
        return;

    CDBCtrl::Instance()->UnInit();
}

bool CGameServer::_InitPlayerMgr()
{
    if(false == CPlayerMgr::Instance()->Init())
        return false;

    return true;
}

void CGameServer::_UnInitPlayerMgr()
{
    if(NULL == CPlayerMgr::Instance())
        return;

    CPlayerMgr::Instance()->UnInit();
}

bool CGameServer::_InitBuddyMgr()
{
    if(false == CBuddyMgr::Instance()->Init())
    {
        return false;
    }

    return true;
}

void CGameServer::_UnInitBuddyMgr()
{
    if(NULL ==CBuddyMgr::Instance())
        return;

    CBuddyMgr::Instance()->UnInit();
}

bool CGameServer::_InitSceneMgr()
{
    if(false == CSceneMgr::Instance()->Init())
        return false;

    return true;
}

void CGameServer::_UnInitSceneMgr()
{
    if(NULL == CSceneMgr::Instance())
        return;

    CSceneMgr::Instance()->UnInit();
}

bool CGameServer::_InitNpcMgr()
{
    return CNpcMgr::Instance()->Init();
}

void CGameServer::_UnInitNpcMgr()
{
    if(NULL == CNpcMgr::Instance())
        return;

    CNpcMgr::Instance()->UnInit();
}

bool CGameServer::_InitMonsterMgr()
{
    return CMonsterMgr::Instance()->Init();
}

void CGameServer::_UnInitMonsterMgr()
{
    if(NULL ==CMonsterMgr::Instance())
    {
        return;
    }

    CMonsterMgr::Instance()->UnInit();
}

bool CGameServer::_InitSkillMgr()
{
    return CSkillMgr::Instance()->Init();
}

void CGameServer::_UnInitSkillMgr()
{
    if(NULL == CSkillMgr::Instance())
    {
        return;
    }

    CSkillMgr::Instance()->UnInit();
}

bool CGameServer::_InitItemMgr()
{
    return CItemMgr::Instance()->Init();
}

bool CGameServer::_InitRaidMgr()
{
    return CRaidMgr::Instance()->Init();
}

bool CGameServer::_InitYunWeiMgr()
{
	return CYunWeiMgr::Instance()->Init();
}

void CGameServer::_UnInitItemMgr()
{
    CItemMgr::Instance()->Uninit();
}

bool CGameServer::_InitGMCmdMgr()
{
    return CGMCmdMgr::Instance()->Init();
}

bool CGameServer::_InitChatMgr()
{
    return CChatMgr::Instance()->Init();
}

void CGameServer::_UnInitGMCmdMgr()
{
    CGMCmdMgr::Instance()->Uninit();
}

bool CGameServer::_InitPropAddonMgr()
{
    return true;
}

void CGameServer::_UnInitPropAddonMgr()
{
}

void CGameServer::SavingData()
{
    CPlayerMgr::Instance()->SavingData();
    CBuddyMgr::Instance()->SaveAllBuddy();
    CBuddyMgr::Instance()->DelAllBuddy();
    CBoardMgr::Instance()->SavingData();
    CGuildMgr::Instance()->SavingData();
    CActivityMgr::Instance()->SaveAllActivity();
    CChatMgr::Instance()->SaveAllLoudSpeak();
    CDBCtrl::Instance()->SaveSysMail();
    CTeamMgr::Instance()->SavingData();

    while(CDBCtrl::Instance()->m_dwCurDBCmdCount != 0)
    {
        CDBCtrl::Instance()->UPdate();
        LVSYS_Sleep(1);
    }

    DBG("CGameServer::SavingData Finished");
}

void CGameServer::MainLoop()
{
    SLVTimeVal stNow;

    LV_STACK_TRACK_ENQUEUE
    while(!m_bTerminate)
    {
        LVSYS_GetTimeEx(&stNow);

        m_dwCurFrame = LV_MSPass(m_stLastUpdateByFrame, stNow);
        if(m_dwCurFrame > m_dwMaxFrame)
        {
            m_dwMaxFrame = m_dwCurFrame;
        }

        if(m_dwCurFrame >= 10)
        {
            m_dwFrameCountOver1++;
        }

        if(m_dwCurFrame >= 20)
        {
            m_dwFrameCountOver2++;
        }

        if(LV_MSPass(m_stLastUpdateByHour, stNow) > 3600 * 1000)
        {
            m_stLastUpdateByHour = stNow;
            m_dwLastMaxFrameByHour = m_dwCurFrame;
        }
        else
        {
            if(m_dwCurFrame > m_dwLastMaxFrameByHour)
            {
                m_dwLastMaxFrameByHour = m_dwCurFrame;
            }
        }

        //DBG("m_dwLastMaxFrameByHour %u", m_dwLastMaxFrameByHour);
        m_stLastUpdateByFrame = stNow;

        CNetCtrl::Instance()->Update();

        SLVTimeVal stStartTime;
        SLVTimeVal stEndTime;
        UINT32 dwInterval = 0;

        LVSYS_GetTimeEx(&stStartTime);
        CDBCtrl::Instance()->UPdate();
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CDBCtrl Update too long... Interval %u ms", dwInterval);
        } 

        LVSYS_GetTimeEx(&stStartTime);
        CGuildBattleMgr::Instance()->Update(stNow.m_dwSec);
        CPlayerMgr::Instance()->Update(stNow);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CPlayerMgr Update too long... Interval %u ms", dwInterval);
        } 

        LVSYS_GetTimeEx(&stStartTime);
        CSlaveHandler::Instance()->Update(stNow);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CSlaveHandler Update too long... Interval %u ms", dwInterval);
        } 

        LVSYS_GetTimeEx(&stStartTime);
        CMapVideoMgr::Instance()->Update(stNow.m_dwSec);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CMapVideoMgr Update too long... Interval %u ms", dwInterval);
        } 

        LVSYS_GetTimeEx(&stStartTime);
        CBoardMgr::Instance()->Update(stNow);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CBoardMgr Update too long... Interval %u ms", dwInterval);
        } 

        LVSYS_GetTimeEx(&stStartTime);
        CTradeMgr::Instance()->Update(stNow.m_dwSec);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CTradeMgr Update too long... Interval %u ms", dwInterval);
        } 

		LVSYS_GetTimeEx(&stStartTime);
		CYunWeiMgr::Instance()->Update(stNow.m_dwSec);
		LVSYS_GetTimeEx(&stEndTime);
		dwInterval = LV_MSPass(stStartTime, stEndTime);
		if(dwInterval > 100)
		{
			CRI("CSlaveHandler Update too long... Interval %u ms", dwInterval);
		}

        LVSYS_GetTimeEx(&stStartTime);
        CDBCtrl::Instance()->UpdateSysMail(stNow.m_dwSec);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CDBCtrl UpdateSysMail too long... Interval %u ms", dwInterval);
        }

        LVSYS_GetTimeEx(&stStartTime);
        CUnitSceneMgr::Instance()->Update(stNow);
        LVSYS_GetTimeEx(&stEndTime);
        dwInterval = LV_MSPass(stStartTime, stEndTime);
        if(dwInterval > 100)
        {
            CRI("CUnitSceneMgr RefreshUnit too long... Interval %u ms", dwInterval);
        }

		LVSYS_GetTimeEx(&stStartTime);
		CTeamMgr::Instance()->Update(stNow.m_dwSec);
		LVSYS_GetTimeEx(&stEndTime);
		dwInterval = LV_MSPass(stStartTime, stEndTime);
		if(dwInterval > 100)
		{
			CRI("CTeamMgr Update too long... Interval %u ms", dwInterval);
		}

        LVSYS_Sleep(1);
    }
}

void CGameServer::_TestKeyWord()
{
    char buf[1024*1024];
    FILE* fp = fopen("chkfile.txt", "rb");
    if(NULL == fp)
        return;

    UINT64 qwSize = fread(buf, 1, sizeof(buf), fp);

    if(m_oWordFilter.FindKeyWord(buf, qwSize))
    {
        printf("Find Key Word!\n");
    }
    else
    {
        printf("Not Find\n");
    }
}

void CGameServer::_TestTable()
{
    vector<string> oVecLines;

    if(false == GetAllLinesFromTblFile("testlines.txt", oVecLines))
    {
        printf("GetAllLinesFromTblFile testlines.txt failed\n");
        return;
    }

    for(size_t i = 0; i < oVecLines.size(); i++)
    {
        vector<string> oVecSegs;
        SplitString(oVecLines[i].c_str(), '\t', oVecSegs);
        printf("Line %zd: ", i);
        for(size_t n = 0; n < oVecSegs.size(); n++)
        {
            printf("%s\t", oVecSegs[n].c_str());
        }
        printf("\n");
    }
}

bool CGameServer::_InitMapHistoryMgr()
{
    CMapVideoMgr::Instance()->Init();

    return true;
}

void CGameServer::_UnInitMapHistoryMgr()
{
}

bool CGameServer::_InitShopMgr()
{
    CShopMgr::Instance()->InitShopMgr();

    return true;
}

void CGameServer::_UnInitShopMgr()
{
}

bool CGameServer::_InitCGuildMgr()
{
    return CGuildMgr::Instance()->Init();
}

void CGameServer::_UnInitCGuildMgr()
{
}

bool CGameServer::_InitBoardMgr()
{
    return CBoardMgr::Instance()->Init();
}

void CGameServer::_UnInitBoardMgr()
{
    CBoardMgr::Instance()->Uninit();
}

bool CGameServer::_InitScheduleMgr()
{
    CScheduleMgr::Instance()->Init();

    return true;
}

void CGameServer::_UnInitScheduleMgr()
{
}

bool CGameServer::_InitActivityMgr()
{
    return(CActivityMgr::Instance()->Init());
}

void CGameServer::_UnInitActivityMgr()
{
    CActivityMgr::Instance()->UnInit();
}

bool CGameServer::_InitGuildBattleMgr()
{
    return CGuildBattleMgr::Instance()->Init();
}

bool CGameServer::_InitArenaMgr()
{
    return CArenaMgr::Instance()->Init();
}

void CGameServer::_UnInitArenaMgr()
{
    CArenaMgr::Instance()->UnInit();
}

bool CGameServer::_InitShmmgr()
{
    return CShmMgr::Instance()->Init();
}

void CGameServer::_UnInitShmmgr()
{
    CShmMgr::Instance()->Uninit();
    return;
}

void CGameServer::_UnInitYunWeiMgr()
{
	CYunWeiMgr::Instance()->UnInit();
}

void CGameServer::_UnInitChatMgr()
{
    CChatMgr::Instance()->Uninit();
}

bool CGameServer::_InitMailMgr()
{
    return CMailMgr::Instance()->Init();
}

void CGameServer::_UnInitMailMgr()
{
    CMailMgr::Instance()->UnInit();
}

bool CGameServer::_InitUnitSceneMgr()
{
    return CUnitSceneMgr::Instance()->Init();
}

void CGameServer::_UnInitUnitSceneMgr()
{
    CUnitSceneMgr::Instance()->UnInit();
}

bool CGameServer::_InitTeamMgr()
{
	return CTeamMgr::Instance()->Init();
}

void CGameServer::_UnInitTeamMgr()
{
	CTeamMgr::Instance()->UnInit();
}

bool CGameServer::_InitDoodadMgr()
{
    return CDoodadMgr::Instance()->Init();
}

void CGameServer::_UnInitDoodadMgr()
{
    CDoodadMgr::Instance()->UnInit();
}

bool CGameServer::_InitTradeMgr()
{
    if(false == CTradeMgr::Instance()->Init())
    {
        return false;
    }

    return true;
}

void CGameServer::_UnInitTradeMgr()
{
    CTradeMgr::Instance()->UnInit();
    return;
}

bool CGameServer::_InitTaskMgr()
{
    if(false == CTaskMgr::Instance()->Init())
    {
        return false;
    }

    return true;
}

void CGameServer::_UnInitTaskMgr()
{
    CTaskMgr::Instance()->UnInit();
    return;
}

bool CGameServer::_LoadTableValue()
{

    return true;
}

bool CGameServer::_CheckTables()
{
	//check market table
	CTblMarket::CMapItem& roMapItems = g_oTblMarket.GetAllItems();
	CTblMarket::CMapItem::iterator it = roMapItems.begin();

	CTblBaoShi::CItem* poBaoshi = NULL;
	CTblItem::CItem* poItem = NULL; 
	while(it != roMapItems.end())
	{
		poItem = g_oTblItem.Get(it->first);

		if( NULL == poItem )
		{
			poBaoshi = g_oTblBaoShi.Get( it->first );
			if( NULL == poBaoshi )
			{
				CRI("CGameServer::_CheckTables, market table find wrong itemid %u", it->first);
				return false;
			}
		}

		++it;
	}
	
	return true;
}









