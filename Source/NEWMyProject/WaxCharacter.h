#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "WaxCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UDimensionManagerComponent;
class UInputMappingContext;
class UInputAction;
class AMagic_FireRing;

UCLASS()
class NEWMYPROJECT_API AWaxCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AWaxCharacter();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // --- 增强输入系统 (Enhanced Input) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> SwitchDimensionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> CastMagicAction;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void SwitchDimension(const FInputActionValue& Value);
    void CastMagic(const FInputActionValue& Value);

protected:
    // --- 核心组件 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> CameraComponent;

    // 维度切换逻辑封装
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension")
    TObjectPtr<UDimensionManagerComponent> DimensionManager;

    // 施法时生成的火环魔法类（在蓝图中指定为 BP_Magic_FireRing）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    TSubclassOf<AMagic_FireRing> MagicFireRingClass;

    // 预留机制接口：环境感知模块（适配昼夜/光照/温度机制）
    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    // TObjectPtr<UEnvironmentSensorComponent> EnvironmentSensor;
};
