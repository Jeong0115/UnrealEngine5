#pragma once

#define WITH_SERVER_CODE_EDITOR if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
#define WITH_SERVER_CODE_EDITOR_ENDIF }

#define GetInfoSubsystem(InfoSubsytemClass) GetGameInstance()->GetSubsystem<InfoSubsytemClass>()
#define GetInfoSubsystemWorld(InfoSubsytemClass) GetWorld()->GetGameInstance()->GetSubsystem<InfoSubsytemClass>()

#define GetRPGWorldGameInstance() Cast<URPGWorldGameInstance>(GetWorld()->GetGameInstance())


#if DO_CHECK
	#define ensureMsgfDebug(InExpression, InFormat, ... ) ensureMsgf(InExpression, InFormat, ##__VA_ARGS__)
#else
	#define ensureMsgfDebug(InExpression, InFormat, ... )
#endif

#define CheckTime(Result, Funcs) \
	double startTime = FPlatformTime::Seconds(); \
	Funcs; \
	double lastTime = FPlatformTime::Seconds(); \
	double Result = lastTime - startTime


#define BossMonsterKey			1000
#define RevivalScrollItemKey	5

#define NotifyZOrder			5
#define RespawnZOrder			4
#define InvnetoryZOrder			3
#define StoreZOrder				2
#define QuestZOrder				1