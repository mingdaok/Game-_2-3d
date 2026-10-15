#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magic_FireRing.generated.h"

class UBoxComponent;
class USceneComponent;
class UNiagaraComponent;

/**
 * 火元素+圈形态 的魔法实体。
 * 根据当前维度（3D/2D）动态切换碰撞形态和粒子特效：
 *   - 3D 模式：宽而扁的环形碰撞 + FX_Fire3D 特效
 *   - 2D 模式：窄而高的火墙碰撞 + FX_Fire2D 特效
 * 
 * 使用方式：SpawnActor 后调用 InitializeMagic(bIs2DMode)，
 * C++ 会自动激活对应的碰撞体和 Niagara 特效。
 */
UCLASS(Blueprintable)
class NEWMYPROJECT_API AMagic_FireRing : public AActor
{
    GENERATED_BODY()

public:
    AMagic_FireRing();

    // --- 核心接口 ---

    /** 
     * 初始化魔法实体，根据维度模式激活对应碰撞体、特效，并启动自毁倒计时。
     * 应在 SpawnActor 之后立刻调用。
     */
    UFUNCTION(BlueprintCallable, Category = "Magic|FireRing")
    void InitializeMagic(bool bIs2DMode);

    /** 
     * 蓝图实现事件：当魔法被激活时触发。
     * 可在蓝图子类中 Override 此事件来添加额外的自定义逻辑（音效等）。
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Magic|FireRing")
    void OnMagicActivated(bool bIs2D);

protected:
    virtual void BeginPlay() override;

    // --- 碰撞组件 ---

    /** 空根组件，作为整体变换的锚点 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Components")
    TObjectPtr<USceneComponent> RootScene;

    /** 3D 模式下的环形碰撞区域（宽而扁） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Components")
    TObjectPtr<UBoxComponent> Collision3D_Ring;

    /** 2D 模式下的火墙碰撞区域（窄而高） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Components")
    TObjectPtr<UBoxComponent> Collision2D_Wall;

    // --- Niagara 特效组件（在蓝图子类中设置具体的 Niagara 资产） ---

    /** 3D 模式下的火环粒子特效 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|FX")
    TObjectPtr<UNiagaraComponent> FX_Fire3D;

    /** 2D 模式下的火墙粒子特效 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|FX")
    TObjectPtr<UNiagaraComponent> FX_Fire2D;

    // --- 可调参数 ---

    /** 3D 环形碰撞的半范围 (XYZ 半径) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Config")
    FVector Ring3DExtent;

    /** 2D 火墙碰撞的半范围 (XYZ 半径) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Config")
    FVector Wall2DExtent;

    /** 魔法存在时长（秒），到期后自动销毁 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Config")
    float LifeSpanSeconds;

private:
    /** 自毁定时器句柄 */
    FTimerHandle DestroyTimerHandle;

    /** 定时器回调：执行自毁 */
    void OnLifeSpanExpired();

    /** 整个 Actor 发生重叠时的原生回调（比组件委托更稳定，不受Live Coding宏注册影响） */
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    // --- [预留功能] 冷却机制变量 (后期完善时取消注释) ---
    // 记录已触发过交互的实体及其上次受击时间
    // TMap<TWeakObjectPtr<AActor>, float> HitCooldownMap;
    
    // 每个实体受到伤害/触发的冷却时间（秒）
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Config", meta = (AllowPrivateAccess = "true"))
    // float HitCooldownSeconds = 1.0f;
};
