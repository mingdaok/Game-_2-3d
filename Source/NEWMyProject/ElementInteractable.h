#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ElementInteractable.generated.h"

// 魔法元素枚举
UENUM(BlueprintType)
enum class EMagicElement : uint8
{
    None  UMETA(DisplayName = "None"),
    Fire  UMETA(DisplayName = "Fire"),
    Ice   UMETA(DisplayName = "Ice"),
    Light UMETA(DisplayName = "Light")
};

// 接口的 UObject 包装类，UE5 反射系统使用，不在这里写业务逻辑
UINTERFACE(MinimalAPI, BlueprintType)
class UElementInteractable : public UInterface
{
    GENERATED_BODY()
};

/**
 * 元素交互接口，供需要响应魔法和环境光照的 Actor（如蜡人主角、环境物体）继承实现
 */
class NEWMYPROJECT_API IElementInteractable
{
    GENERATED_BODY()

    // 将接口函数添加在此类中。需要支持此接口的类将继承该类。
public:

    // 当被魔法元素击中时触发
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MagicInteraction")
    void OnHitByMagic(EMagicElement ElementType);

    // 当受到环境光照照射时触发
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MagicInteraction")
    void OnIlluminated(float LightIntensity);
};
