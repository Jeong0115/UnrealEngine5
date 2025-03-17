
#include "RPGWorldGameInstance.h"

#include "Defines.h"

#include "DB\Public\WrapperDB.h"

#include "BasePlayerController.h"


void URPGWorldGameInstance::Init()
{
	Super::Init();
WITH_SERVER_CODE_EDITOR


	WrapperDB::ConnectToDatabase();

WITH_SERVER_CODE_EDITOR_ENDIF

}