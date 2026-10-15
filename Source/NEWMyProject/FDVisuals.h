#pragma once
#include "CoreMinimal.h"
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
namespace FDVisuals
{
    UMaterialInstanceDynamic* Material(UObject* Owner, FLinearColor Color, float Glow = 0);
    void Sound(AActor* Owner, const TCHAR* Name, float Volume = .5f);
    void BuildScenery(AActor* Owner, USceneComponent* Parent, int32 Room);
    UStaticMeshComponent* Piece(AActor* Owner, USceneComponent* Parent, const TCHAR* Shape, FVector Position, FVector Size, FLinearColor Color, float Glow = 0);
    const FLinearColor Stone(.24f, .32f, .33f);
    const FLinearColor Edge(.48f, .67f, .61f);
    const FLinearColor Gold(.95f, .54f, .16f);
    const FLinearColor Ink(.016f, .028f, .038f);
}
