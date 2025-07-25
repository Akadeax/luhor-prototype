// Fill out your copyright notice in the Description page of Project Settings.


#include "BPComponentUtil.h"

UObject* UBPComponentUtil::GetComponentOfClassWithTag(AActor* Actor, TSubclassOf<UObject> Class, FName Tag)
{
	if (!Actor) return nullptr;
	
	for (UActorComponent* comp : Actor->GetComponents())
	{
		if (comp->IsA(Class) && comp->ComponentHasTag(Tag)) return comp;
	}

	return nullptr;
}
