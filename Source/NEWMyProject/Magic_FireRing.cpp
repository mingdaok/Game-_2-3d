#include "Magic_FireRing.h"
#include "Components/BoxComponent.h"
#include "ElementInteractable.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AMagic_FireRing::AMagic_FireRing()
{
    PrimaryActorTick.bCanEverTick = false;

    // --- 默认碰撞尺寸参数 ---
    Ring3DExtent = FVector(200.0f, 200.0f, 30.0f);  // 宽而扁的环
    Wall2DExtent = FVector(50.0f, 50.0f, 200.0f);   // 窄而高的火墙
    LifeSpanSeconds = 3.0f;

    // --- 组件层级构建 ---

    // 空根组件，作为整体变换的锚点
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    SetRootComponent(RootScene);

    // 3D 环形碰撞体（默认关闭，等待 InitializeMagic 激活）
    Collision3D_Ring = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision3D_Ring"));
    Collision3D_Ring->SetupAttachment(RootScene);
    Collision3D_Ring->SetBoxExtent(Ring3DExtent);
    Collision3D_Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Collision3D_Ring->SetCollisionResponseToAllChannels(ECR_Overlap);
    Collision3D_Ring->SetGenerateOverlapEvents(true);
    Collision3D_Ring->SetHiddenInGame(true);

    // 2D 火墙碰撞体（默认关闭，等待 InitializeMagic 激活）
    Collision2D_Wall = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision2D_Wall"));
    Collision2D_Wall->SetupAttachment(RootScene);
    Collision2D_Wall->SetBoxExtent(Wall2DExtent);
    Collision2D_Wall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Collision2D_Wall->SetCollisionResponseToAllChannels(ECR_Overlap);
    Collision2D_Wall->SetGenerateOverlapEvents(true);
    Collision2D_Wall->SetHiddenInGame(true);

    // 3D 火环特效组件（默认不自动播放，由 InitializeMagic 控制）
    FX_Fire3D = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FX_Fire3D"));
    FX_Fire3D->SetupAttachment(RootScene);
    FX_Fire3D->bAutoActivate = false;

    // 2D 火墙特效组件（默认不自动播放，由 InitializeMagic 控制）
    FX_Fire2D = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FX_Fire2D"));
    FX_Fire2D->SetupAttachment(RootScene);
    FX_Fire2D->bAutoActivate = false;
}

void AMagic_FireRing::BeginPlay()
{
    Super::BeginPlay();
}

void AMagic_FireRing::InitializeMagic(bool bIs2DMode)
{
    // 根据编辑器中可能修改过的参数，刷新碰撞体尺寸
    Collision3D_Ring->SetBoxExtent(Ring3DExtent);
    Collision2D_Wall->SetBoxExtent(Wall2DExtent);

    if (bIs2DMode)
    {
        // 2D 模式：激活火墙碰撞 + 火墙特效，关闭环形碰撞 + 环形特效
        Collision3D_Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Collision3D_Ring->SetActive(false);
        FX_Fire3D->Deactivate();

        Collision2D_Wall->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Collision2D_Wall->SetActive(true);
        FX_Fire2D->Activate();

        // 调试可视化：绿色线框代表 2D 火墙
        DrawDebugBox(GetWorld(), GetActorLocation(), Wall2DExtent, FColor::Green, false, LifeSpanSeconds, 0, 3.0f);
        UE_LOG(LogTemp, Warning, TEXT("[FireRing] 2D fire wall spawned at %s"), *GetActorLocation().ToString());
    }
    else
    {
        // 3D 模式：激活环形碰撞 + 环形特效，关闭火墙碰撞 + 火墙特效
        Collision2D_Wall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Collision2D_Wall->SetActive(false);
        FX_Fire2D->Deactivate();

        Collision3D_Ring->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Collision3D_Ring->SetActive(true);
        FX_Fire3D->Activate();

        // 调试可视化：橙色线框代表 3D 火环
        DrawDebugBox(GetWorld(), GetActorLocation(), Ring3DExtent, FColor::Orange, false, LifeSpanSeconds, 0, 3.0f);
        UE_LOG(LogTemp, Warning, TEXT("[FireRing] 3D fire ring spawned at %s"), *GetActorLocation().ToString());
    }

    // 通知蓝图侧（可用于播放音效等额外逻辑）
    OnMagicActivated(bIs2DMode);

    // 启动自毁倒计时
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DestroyTimerHandle,
            this,
            &AMagic_FireRing::OnLifeSpanExpired,
            LifeSpanSeconds,
            false  // 不循环，只执行一次
        );
    }
}

void AMagic_FireRing::OnLifeSpanExpired()
{
    Destroy();
}

void AMagic_FireRing::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    // 如果碰到的是有效的Actor，并且不是魔法自身
    if (OtherActor && OtherActor != this)
    {
        // 增加底层日志，确认碰撞是否真的触发了
        UE_LOG(LogTemp, Error, TEXT("[FireRing DEBUG] 底层重叠已触发！碰到了: %s"), *OtherActor->GetName());

        // 判断对方是否继承了 ElementInteractable 接口
        if (OtherActor->Implements<UElementInteractable>())
        {
            // 执行接口函数，发送 Fire 元素击中的信号
            IElementInteractable::Execute_OnHitByMagic(OtherActor, EMagicElement::Fire);
            
            UE_LOG(LogTemp, Warning, TEXT("[FireRing] 击中了具有接口的实体: %s"), *OtherActor->GetName());
        }
    }
}
