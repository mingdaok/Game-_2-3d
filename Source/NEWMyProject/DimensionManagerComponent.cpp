#include "DimensionManagerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UDimensionManagerComponent::UDimensionManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 默认起始状态
    CurrentDimensionState = EDimensionState::Dimension3D;
    
    // 视角默认参数
    SpringArmLength3D = 400.0f;
    SpringArmLength2D = 800.0f;
    FieldOfView3D = 90.0f;
    FieldOfView2D = 35.0f; 
    OrthoWidth2D = 1200.0f; // 正交视角的默认宽度
    MaxProjectableSize = 1000.0f; // 默认最大投影尺寸 (10米)
}

void UDimensionManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerSpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
        OwnerCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
    }

    // 初始化应用
    ApplyDimensionSettings();
}

void UDimensionManagerComponent::ToggleDimension()
{
    if (CurrentDimensionState == EDimensionState::Dimension3D)
    {
        SetDimensionState(EDimensionState::Dimension2D);
    }
    else
    {
        SetDimensionState(EDimensionState::Dimension3D);
    }
}

void UDimensionManagerComponent::SetDimensionState(EDimensionState NewState)
{
    if (CurrentDimensionState != NewState)
    {
        CurrentDimensionState = NewState;
        ApplyDimensionSettings();
        OnDimensionSwitchedDelegate.Broadcast(CurrentDimensionState);
    }
}

void UDimensionManagerComponent::ApplyDimensionSettings()
{
    if (!OwnerCharacter || !OwnerSpringArm || !OwnerCamera) return;

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();

    if (CurrentDimensionState == EDimensionState::Dimension3D)
    {
        // 先还原深度，再清理碰撞体，并恢复被隐藏的外墙
        Recover3DDepth();
        Clear2DCollisions();
        RestoreOccluders();

        // 恢复3D视角：透视投影
        OwnerCamera->SetProjectionMode(ECameraProjectionMode::Perspective);
        OwnerCamera->SetFieldOfView(FieldOfView3D);
        OwnerSpringArm->TargetArmLength = SpringArmLength3D;
        
        // 强制同步控制器旋转，防止切回3D时镜头瞬间跳跃
        if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
        {
            PC->SetControlRotation(OwnerSpringArm->GetComponentRotation());
        }
        OwnerSpringArm->bUsePawnControlRotation = true; 
        OwnerSpringArm->SetUsingAbsoluteRotation(false); // 恢复相对旋转

        // 解除平面约束，允许Z/Y轴自由移动
        if (MovementComp)
        {
            MovementComp->bConstrainToPlane = false;
            MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 恢复3D平滑转身
        }
    }
    else if (CurrentDimensionState == EDimensionState::Dimension2D)
    {
        // 抓取当前控制器的Yaw并吸附到最近的90度
        if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
        {
            float CurrentYaw = PC->GetControlRotation().Yaw;
            float SnappedYaw = FMath::RoundToFloat(CurrentYaw / 90.0f) * 90.0f;
            Saved2DYawRotation = FRotator(0.0f, SnappedYaw, 0.0f);
        }
        else
        {
            Saved2DYawRotation = FRotator(0.0f, -90.0f, 0.0f); // 默认后备
        }

        // 应用真2D视角：正交投影
        OwnerCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        OwnerCamera->SetOrthoWidth(OrthoWidth2D);
        OwnerSpringArm->TargetArmLength = SpringArmLength2D;
        
        // 固定视角：绝对旋转，永远沿锁定的动态切面看向角色
        OwnerSpringArm->bUsePawnControlRotation = false;
        OwnerSpringArm->SetUsingAbsoluteRotation(true); 
        OwnerSpringArm->SetWorldRotation(Saved2DYawRotation); 

        // 启用平面约束，物理层面锁定动态法线面
        if (MovementComp)
        {
            MovementComp->bConstrainToPlane = true;
            MovementComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Custom);
            MovementComp->SetPlaneConstraintNormal(Saved2DYawRotation.Vector());
            MovementComp->RotationRate = FRotator(0.0f, 50000.0f, 0.0f); // 瞬间转身消除转圈感

            // 瞬间强制角色朝向动态的左或右
            FVector CamRight = FRotationMatrix(Saved2DYawRotation).GetUnitAxis(EAxis::Y);
            float Dot = FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), CamRight);
            if (Dot < 0)
            {
                OwnerCharacter->SetActorRotation((-CamRight).Rotation()); // 朝左
            }
            else
            {
                OwnerCharacter->SetActorRotation(CamRight.Rotation()); // 朝右
            }
        }

        // 自动降维打击，生成物理投影垫脚石
        Generate2DCollisions();

        // 智能隐藏挡住镜头的密闭房间外墙
        HandleCameraOcclusion();
    }
}
void UDimensionManagerComponent::Generate2DCollisions()
{
    Clear2DCollisions();

    if (!GetWorld() || !OwnerCharacter) return;

    FVector Fwd = Saved2DYawRotation.Vector();
    FVector FwdAbs = Fwd.GetAbs();
    
    // 平面轴向 (例如：如果 Fwd 是 X 轴，那么平面轴就是 Y 和 Z)
    FVector PlaneAxis = FVector(1.0f, 1.0f, 1.0f) - FwdAbs;
    PlaneAxis.X = FMath::RoundToInt(PlaneAxis.X);
    PlaneAxis.Y = FMath::RoundToInt(PlaneAxis.Y);
    PlaneAxis.Z = FMath::RoundToInt(PlaneAxis.Z);

    FVector PlayerLoc = OwnerCharacter->GetActorLocation();

    // 生成隐藏的物理容器 Actor，以最标准的格式被 UE5 物理引擎接管
    ProjectedCollisionActor = GetWorld()->SpawnActor<AActor>();
    USceneComponent* RootComp = NewObject<USceneComponent>(ProjectedCollisionActor);
    RootComp->RegisterComponent();
    ProjectedCollisionActor->SetRootComponent(RootComp);

    // 遍历世界上所有的 Actor
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        // 排除主角自身和当前容器
        if (!Actor || Actor == OwnerCharacter.Get() || Actor == GetOwner() || Actor == ProjectedCollisionActor) continue;

        // 仅提取 UStaticMeshComponent，这自动过滤掉了不可见体积(Volumes)和摄像机等杂物！
        TArray<UStaticMeshComponent*> SMCs;
        Actor->GetComponents<UStaticMeshComponent>(SMCs);

        for (UStaticMeshComponent* SMC : SMCs)
        {
            if (SMC && SMC->GetCollisionEnabled() != ECollisionEnabled::NoCollision && 
                SMC->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
            {
                FBoxSphereBounds Bounds = SMC->Bounds;
                
                // 巨型布景过滤 (Scenery Filter)：排除超大体积的环境建筑
                if (Bounds.BoxExtent.GetMax() > MaxProjectableSize)
                {
                    continue; // 它是风景/房间外墙，不参与 2D 物理判定，也不生成投影
                }

                FVector Origin = Bounds.Origin;

                // 投影到主角切面上：抹除深度方向的原坐标，替换为主角在深度方向的坐标
                FVector ProjectedOrigin = Origin * PlaneAxis + PlayerLoc * FwdAbs;

                // 拍扁深度方向
                FVector Extent = Bounds.BoxExtent;
                if (FwdAbs.X > 0.5f) Extent.X = 50.0f; 
                if (FwdAbs.Y > 0.5f) Extent.Y = 50.0f;

                // 在物理容器上生成组件
                UBoxComponent* NewBox = NewObject<UBoxComponent>(ProjectedCollisionActor);
                NewBox->CreationMethod = EComponentCreationMethod::Instance;
                NewBox->SetupAttachment(RootComp);
                
                // 必须在注册前设置物理属性，确保引擎承认
                NewBox->SetBoxExtent(Extent);
                NewBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                NewBox->SetCollisionProfileName(TEXT("BlockAll"));
                NewBox->SetHiddenInGame(true);

                // 注册并设置绝对位置
                NewBox->RegisterComponent();
                NewBox->SetWorldLocation(ProjectedOrigin);
                NewBox->SetWorldRotation(FRotator::ZeroRotator);

                // 动态幽灵碰撞技术 (Contextual Ghosting)
                // 检查这个刚刚生成的方块是否会"挤开"玩家
                if (IsPlayerOverlappingBox(ProjectedOrigin, Extent))
                {
                    // 将其设为对 Pawn 穿透，这样玩家就不会被挤开，而是和它完美重合！
                    NewBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
                    OverlappedProjectedBoxes.Add(NewBox);

                    // 启动高频定时器监控幽灵方块（完全不依赖TickComponent，彻底绕开蓝图序列化陷阱！）
                    if (!GhostCheckTimerHandle.IsValid())
                    {
                        GetWorld()->GetTimerManager().SetTimer(
                            GhostCheckTimerHandle,
                            this,
                            &UDimensionManagerComponent::CheckGhostBoxes,
                            0.016f,  // 约60fps的检测频率
                            true     // 循环执行
                        );
                    }
                }

                // 绘制调试红色线框 (持续10秒，厚度5)
                DrawDebugBox(GetWorld(), ProjectedOrigin, Extent, FColor::Red, false, 10.0f, 0, 5.0f);

                FProjectedCollision Proj;
                Proj.Box = NewBox;
                Proj.OriginalDepthCenter = Origin;
                ProjectedCollisions.Add(Proj);
            }
        }
    }
}

void UDimensionManagerComponent::Clear2DCollisions()
{
    OverlappedProjectedBoxes.Empty();
    // 清除幽灵监控定时器
    if (GetWorld() && GhostCheckTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(GhostCheckTimerHandle);
    }
    if (ProjectedCollisionActor)
    {
        ProjectedCollisionActor->Destroy();
        ProjectedCollisionActor = nullptr;
    }
    ProjectedCollisions.Empty();
}

bool UDimensionManagerComponent::IsPlayerOverlappingBox(FVector BoxCenter, FVector BoxExtent)
{
    if (!OwnerCharacter) return false;
    
    UCapsuleComponent* Cap = OwnerCharacter->GetCapsuleComponent();
    if (!Cap) return false;

    FVector PlayerLoc = OwnerCharacter->GetActorLocation();
    float CapRadius = Cap->GetUnscaledCapsuleRadius();
    float CapHalfHeight = Cap->GetUnscaledCapsuleHalfHeight();

    // 缩小一点点容错，防止恰好站在上面时也被误判为重叠（导致实体变幽灵掉下去）
    return FMath::Abs(BoxCenter.X - PlayerLoc.X) < (BoxExtent.X + CapRadius - 2.0f) &&
           FMath::Abs(BoxCenter.Y - PlayerLoc.Y) < (BoxExtent.Y + CapRadius - 2.0f) &&
           FMath::Abs(BoxCenter.Z - PlayerLoc.Z) < (BoxExtent.Z + CapHalfHeight - 2.0f);
}

void UDimensionManagerComponent::CheckGhostBoxes()
{
    if (CurrentDimensionState != EDimensionState::Dimension2D)
    {
        // 不在2D模式了，清理定时器
        if (GetWorld() && GhostCheckTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(GhostCheckTimerHandle);
        }
        return;
    }

    // 动态检查幽灵方块
    for (int32 i = OverlappedProjectedBoxes.Num() - 1; i >= 0; --i)
    {
        UBoxComponent* Box = OverlappedProjectedBoxes[i];
        if (!IsValid(Box))
        {
            OverlappedProjectedBoxes.RemoveAt(i);
            continue;
        }

        bool bShouldMakeSolid = false;

        // 条件1：玩家已经完全脱离了这个方块（跳到了上面或者走开了）
        if (!IsPlayerOverlappingBox(Box->GetComponentLocation(), Box->GetScaledBoxExtent()))
        {
            bShouldMakeSolid = true;
        }
        else if (OwnerCharacter)
        {
            // 条件2：宽容判定 (Forgiving Snap)
            // 只要正在下落，并且脚底高过了方块顶面，就强行变为实体！
            // UE5底层的防穿透机制会瞬间把玩家"往上挤"，完美实现登顶！
            float PlayerFeetZ = OwnerCharacter->GetActorLocation().Z - OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            float BoxTopZ = Box->GetComponentLocation().Z + Box->GetScaledBoxExtent().Z;
            
            if (OwnerCharacter->GetVelocity().Z < 0.0f && PlayerFeetZ > BoxTopZ - 10.0f)
            {
                bShouldMakeSolid = true;
            }
        }

        if (bShouldMakeSolid)
        {
            // 瞬间把它变回实体方块！
            Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            OverlappedProjectedBoxes.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("[DimensionMgr] Ghost box solidified! Remaining ghosts: %d"), OverlappedProjectedBoxes.Num());
        }
    }

    // 如果所有幽灵方块都已实体化，停止定时器
    if (OverlappedProjectedBoxes.Num() == 0 && GetWorld() && GhostCheckTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(GhostCheckTimerHandle);
    }
}

void UDimensionManagerComponent::HandleCameraOcclusion()
{
    if (!OwnerCharacter || !GetWorld() || !OwnerCamera) return;

    FVector CameraDir = Saved2DYawRotation.Vector().GetSafeNormal();
    FVector PlayerLoc = OwnerCharacter->GetActorLocation();
    FVector CameraLoc = OwnerCamera->GetComponentLocation();

    const FVector LineStart = CameraLoc;
    const FVector LineEnd = PlayerLoc;
    const FVector LineDelta = LineEnd - LineStart;
    const float LineLength = LineDelta.Size();
    if (LineLength <= KINDA_SMALL_NUMBER) return;

    const FVector LineDir = LineDelta / LineLength;
    const float MinOccluderHalfExtent = 100.0f;
    const float ExtraRadius = 30.0f; // 防止近距离误判

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == OwnerCharacter || Actor == ProjectedCollisionActor) continue;
        if (Actor->IsHidden()) continue;

        FVector Origin, Extent;
        Actor->GetActorBounds(false, Origin, Extent);
        if (Extent.GetMax() < MinOccluderHalfExtent) continue;

        // 计算 Actor 中心到相机-玩家连线的最近点
        FVector ToOrigin = Origin - LineStart;
        float AlongLine = FVector::DotProduct(ToOrigin, LineDir);
        if (AlongLine < 0.0f || AlongLine > LineLength) continue;

        FVector ClosestPoint = LineStart + LineDir * AlongLine;
        float ActorRadius = Extent.Size();
        float DistSq = FVector::DistSquared(Origin, ClosestPoint);
        float Threshold = FMath::Square(ActorRadius + ExtraRadius);

        if (DistSq <= Threshold)
        {
            Actor->SetActorHiddenInGame(true);
            HiddenOccluders.Add(Actor);
            UE_LOG(LogTemp, Warning, TEXT("[DimensionMgr] Hidden occluder: %s (origin=%s, extent_max=%.1f, dist=%.1f)"),
                *Actor->GetName(), *Origin.ToString(), Extent.GetMax(), FMath::Sqrt(DistSq));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[DimensionMgr] HandleCameraOcclusion: hidden %d occluders total"), HiddenOccluders.Num());
}

void UDimensionManagerComponent::RestoreOccluders()
{
    for (AActor* Actor : HiddenOccluders)
    {
        if (Actor)
        {
            Actor->SetActorHiddenInGame(false);
        }
    }
    HiddenOccluders.Empty();
}

void UDimensionManagerComponent::Recover3DDepth()
{
    if (ProjectedCollisions.Num() == 0 || !GetWorld() || !OwnerCharacter) return;

    float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    float PlayerFeetZ = OwnerCharacter->GetActorLocation().Z - CapsuleHalfHeight;

    // 只恢复玩家真正站在其顶面上的投影方块的深度
    // 判断标准：玩家脚底 Z 坐标在方块顶面 Z ± 容差范围内
    const float StandingTolerance = 20.0f;

    UBoxComponent* StandingBox = nullptr;
    FVector BestOriginalDepth = FVector::ZeroVector;
    float MaxDepth = -UE_BIG_NUMBER; // 用于追踪最深的深度 (离摄像机最远)
    FVector CamFwd = Saved2DYawRotation.Vector(); // 获取进入2D时的摄像机朝向

    for (const FProjectedCollision& Proj : ProjectedCollisions)
    {
        if (!IsValid(Proj.Box)) continue;

        float BoxTopZ = Proj.Box->GetComponentLocation().Z + Proj.Box->GetScaledBoxExtent().Z;

        // 检查脚底是否在方块顶面附近
        if (FMath::Abs(PlayerFeetZ - BoxTopZ) < StandingTolerance)
        {
            // 还需要检查水平方向是否在方块范围内
            FVector BoxLoc = Proj.Box->GetComponentLocation();
            FVector BoxExt = Proj.Box->GetScaledBoxExtent();

            FVector PlayerLoc = OwnerCharacter->GetActorLocation();
            bool bWithinX = FMath::Abs(PlayerLoc.X - BoxLoc.X) < BoxExt.X + 30.0f;
            bool bWithinY = FMath::Abs(PlayerLoc.Y - BoxLoc.Y) < BoxExt.Y + 30.0f;

            if (bWithinX && bWithinY)
            {
                // 计算当前方块的深度：沿着摄像机前向向量的点积大小
                // 摄像机前方是正值，值越大代表离摄像机越远（即越靠后）
                float CurrentDepth = FVector::DotProduct(Proj.OriginalDepthCenter, CamFwd);
                
                // 选择深度最大的方块（即离摄像机最远、最靠后的方块）
                if (CurrentDepth > MaxDepth)
                {
                    MaxDepth = CurrentDepth;
                    StandingBox = Proj.Box;
                    BestOriginalDepth = Proj.OriginalDepthCenter;
                }
            }
        }
    }

    if (StandingBox)
    {
        FVector FwdAbs = Saved2DYawRotation.Vector().GetAbs();
        FVector PlaneAxis = FVector(1.0f, 1.0f, 1.0f) - FwdAbs;
        PlaneAxis.X = FMath::RoundToInt(PlaneAxis.X);
        PlaneAxis.Y = FMath::RoundToInt(PlaneAxis.Y);
        PlaneAxis.Z = FMath::RoundToInt(PlaneAxis.Z);

        // 保留主角当前的 XZ/YZ 切面坐标，深度方向恢复为原方块的中心深度
        FVector NewLoc = OwnerCharacter->GetActorLocation() * PlaneAxis + BestOriginalDepth * FwdAbs;
        OwnerCharacter->SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Warning, TEXT("[DimensionMgr] Depth recovered to box at %s"), *BestOriginalDepth.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[DimensionMgr] No standing box found, keeping current position."));
    }
}
