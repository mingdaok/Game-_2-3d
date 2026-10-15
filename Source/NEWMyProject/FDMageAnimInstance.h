#pragma once
#include "Animation/AnimInstance.h"
#include "FDMageAnimInstance.generated.h"

class UAnimSequence;

/** Native blend graph for the imported KayKit rig; capsule movement remains authoritative. */
UCLASS()
class NEWMYPROJECT_API UFDMageAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    UPROPERTY(Transient) TArray<TObjectPtr<UAnimSequence>> Clips;
protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
