#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"

namespace FDebugUtil
{
	template<typename TBoolable>
	FORCEINLINE void QuitCheckf(TBoolable Condition, const TCHAR* Format, ...)
	{
#ifdef WITH_EDITOR
		if (!!Condition) return;

		TCHAR buf[1024];

		va_list pArg;
		va_start(pArg, Format);
		FCString::GetVarArgs(buf, UE_ARRAY_COUNT(buf), Format, pArg);
		va_end(pArg);

		UE_LOG(LogTemp, Error, TEXT("%s"), buf);

		if (IsInGameThread())
		{
			check(GEngine->GetCurrentPlayWorld());
			UKismetSystemLibrary::QuitGame(
				GEngine->GetCurrentPlayWorld(),
				nullptr,
				EQuitPreference::Quit,
				true
			);
		}
		else
		{
			AsyncTask(ENamedThreads::GameThread, [=]
			{
				check(GEngine->GetCurrentPlayWorld());
				UKismetSystemLibrary::QuitGame(
					GEngine->GetCurrentPlayWorld(),
					nullptr,
					EQuitPreference::Quit,
					true
				);
			});
		}
#endif
	}
}
