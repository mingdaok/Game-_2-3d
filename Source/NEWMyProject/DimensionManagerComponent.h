#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "DimensionManagerComponent.generated.h"

class ACharacter;
class UCameraComponent;
class USpringArmComponent;

// 维度状态枚举
UENUM(BlueprintType)
enum class EDimensionState : uint8
{
    Dimension3D UMETA(DisplayName = "3D Dimension"),
    Dimension2D UMETA(DisplayName = "2.5D Dimension")
};

// 维度切换委托（供蓝图绑定表现层逻辑，如环境状态视觉切换、特效、音效等）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDimensionSwitchedSignature, EDimensionState, NewDimensionState);

class UBoxComponent;

// 记录2D投影碰撞体及其原始3D深度的结构体
USTRUCT()
struct FProjectedCollision
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UBoxComponent> Box = nullptr;

    UPROPERTY()
    FVector OriginalDepthCenter = FVector::ZeroVector;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEWMYPROJECT_API UDimensionManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UDimensionManagerComponent();

protected:
    virtual void BeginPlay() override;

public:

    // 切换维度的核心接口
    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void ToggleDimension();

    // 显式设置维度
    UFUNCTION(BlueprintCallable, Category = "Dimension")
    void SetDimensionState(EDimensionState NewState);

    // 获取当前维度
    UFUNCTION(BlueprintPure, Category = "Dimension")
    EDimensionState GetCurrentDimension() const { return CurrentDimensionState; }

    // 获取进入2D瞬间动态锁定的相机方向
    UFUNCTION(BlueprintPure, Category = "Dimension")
    FRotator GetSaved2DYawRotation() const { return Saved2DYawRotation; }

    UPROPERTY(BlueprintAssignable, Category = "Dimension")
    FOnDimensionSwitchedSignature OnDimensionSwitchedDelegate;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension")
    EDimensionState CurrentDimensionState;

    // 记录切入2D瞬间的相机水平角度
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension")
    FRotator Saved2DYawRotation;

    // --- 视角参数配置 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension|Camera")
    float SpringArmLength3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension|Camera")
    float SpringArmLength2D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension|Camera")
    float FieldOfView3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension Settings")
    float FieldOfView2D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension Settings")
    float OrthoWidth2D;

    // 巨型布景过滤阈值 (超过此尺寸的物体被认定为背景墙，不生成物理切面，并在挡视线时隐形)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimension Settings")
    float MaxProjectableSize;

    // 内部引用缓存，符合现代UE5指针规范
    TObjectPtr<ACharacter> OwnerCharacter;
    TObjectPtr<USpringArmComponent> OwnerSpringArm;
    TObjectPtr<UCameraComponent> OwnerCamera;

    // --- 2D 碰撞投影与深度还原机制 ---
    UPROPERTY()
    TArray<FProjectedCollision> ProjectedCollisions;

    UPROPERTY()
    TObjectPtr<AActor> ProjectedCollisionActor;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> HiddenOccluders;

    UPROPERTY()
    TArray<TObjectPtr<UBoxComponent>> OverlappedProjectedBoxes;

    FTimerHandle GhostCheckTimerHandle;
    void CheckGhostBoxes();
    bool IsPlayerOverlappingBox(FVector BoxCenter, FVector BoxExtent);

    void Generate2DCollisions();
    void Clear2DCollisions();
    void Recover3DDepth();
    void HandleCameraOcclusion();
    void RestoreOccluders();

    void ApplyDimensionSettings();
};
